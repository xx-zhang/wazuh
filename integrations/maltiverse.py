# Created by Maltiverse.
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

"""
What is Maltiverse?
###################

Maltiverse works as a broker for Threat intelligence sources that are
aggregated from more than a hundred different Public, Private and Community
sources. Once the data is ingested, the IoC Scoring Algorithm applies a
qualitative classification to the IoC that changes. Finally this data can
be queried in a Threat Intelligence feed that can be delivered to your
Firewalls, SOAR, SIEM, EDR or any other technology.

What does this integration?
###########################

This integration enrichs any alert generated by Wazuh via the Maltiverse API,
inserting new fields in case of match and following the threat taxonomy of the
ECS standard (Elastic Common Squema).

https://www.elastic.co/guide/en/ecs/current/ecs-threat.html

Ipv4, Domain names, Urls and MD5/SHA1 checksums are checked in Maltiverse
platform in order to enrich the original alert with threat Intel information

Installation Guide
##################

Add this to the ossec.conf file, inside <ossec_config></ossec_config> block:

    <integration>
        <name>maltiverse</name>
        <hook_url>https://api.maltiverse.com</hook_url>
        <api_key><YOUR_MALTIVERSE_AUTH_TOKEN></api_key>
        <alert_format>json</alert_format>
    </integration>

And restart Wazuh Manager:

    /etc/init.d/wazuh-manager restart

"""

import json
import hashlib
import ipaddress
import os
from socket import socket, AF_UNIX, SOCK_DGRAM
import sys
import time
from urllib.parse import urlsplit

try:
    import requests
except Exception as e:
    print("No module 'requests' found. Install: pip install requests")
    sys.exit(1)

# Global vars
debug_enabled: bool = False
pwd: str = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
json_alert: dict = {}
now: str = time.strftime("%a %b %d %H:%M:%S %Z %Y")

# Set paths
LOG_FILE: str = f"{pwd}/logs/integrations.log"
SOCKET_ADDR: str = f"{pwd}/queue/sockets/queue"


class Maltiverse:
    """This class is a simplification of maltiverse pypi package."""

    def __init__(
        self, endpoint: str = "https://api.maltiverse.com", auth_token: str = None
    ):
        """Initialize the Maltiverse class.

        Parameters
        ----------
        endpoint : str, optional (default: "https://api.maltiverse.com")
            The API endpoint URL.
        auth_token : str, optional
            The authentication token for the API.
        """
        self.endpoint = endpoint
        self.auth_token = auth_token
        self.headers = {
            "Accept": "application/json",
            "Authorization": f"Bearer {self.auth_token}",
        }

    def ip_get(self, ip_addr: str) -> dict:
        """Request Maltiverse Ipv4 via API.

        Parameters
        ----------
        ip_addr : str
            The IP address to request.

        Returns
        -------
        dict
            The Maltiverse Ipv4 information as a dictionary.
        """
        return requests.get(
            f"{self.endpoint}/ip/{ip_addr}", headers=self.headers
        ).json()

    def hostname_get(self, hostname: str) -> dict:
        """Request Maltiverse hostname via API.

        Parameters
        ----------
        hostname : str
            The hostname to request.

        Returns
        -------
        dict
            The Maltiverse hostname information as a dictionary.
        """
        return requests.get(
            f"{self.endpoint}/hostname/{hostname}", headers=self.headers
        ).json()

    def url_get(self, url: str) -> dict:
        """Request Maltiverse URL via API.

        Parameters
        ----------
        url : str
            The URL to request.

        Returns
        -------
        dict
            The Maltiverse URL information as a dictionary.
        """
        urlchecksum = hashlib.sha256(url.encode("utf-8")).hexdigest()
        return requests.get(
            f"{self.endpoint}/url/{urlchecksum}", headers=self.headers
        ).json()

    def sample_get(self, sample: str, algorithm: str = "md5") -> dict:
        """Request Maltiverse sample via API.

        Parameters
        ----------
        sample : str
            The sample to request.
        algorithm : str, optional (default: "md5")
            The algorithm used for the sample search.

        Returns
        -------
        dict
            The Maltiverse sample information as a dictionary.
        """
        mapping = {
            "md5": self.sample_get_by_md5,
            "sha1": self.sample_get_by_sha1,
        }
        callable_function = mapping.get(algorithm, mapping.get("md5"))
        return callable_function(sample)

    def sample_get_by_md5(self, md5: str):
        """Request Maltiverse MD5 sample via API.

        Parameters
        ----------
        md5 : str
            The MD5 checksum of the sample.

        Returns
        -------
        dict
            The Maltiverse MD5 sample information as a dictionary.
        """
        return requests.get(
            f"{self.endpoint}/sample/md5/{md5}", headers=self.headers
        ).json()

    def sample_get_by_sha1(self, sha1: str):
        """Request Maltiverse SHA1 sample via API.

        Parameters
        ----------
        sha1 : str
            The SHA1 checksum of the sample.

        Returns
        -------
        dict
            The Maltiverse SHA1 sample information as a dictionary.
        """
        return requests.get(
            f"{self.endpoint}/sample/sha1/{sha1}", headers=self.headers
        ).json()


def is_valid_url(url: str) -> bool:
    """Check if a URL is valid.

    Parameters
    ----------
    url : str
        The URL to check.

    Returns
    -------
    bool
        True if the URL is valid, False otherwise.
    """
    split_url = urlsplit(url)
    return bool(split_url.scheme and split_url.netloc)


def main(args: list):
    """The main entry point of the script.

    Parameters
    ----------
    args : list
        The command-line arguments passed to the script.
    """
    global debug_enabled
    try:
        # Read arguments
        bad_arguments = False
        if len(args) >= 4:
            msg = "{0} {1} {2} {3} {4}".format(
                now,
                args[1],
                args[2],
                args[3],
                args[4] if len(args) > 4 else "",
            )
            debug_enabled = len(args) > 4 and args[4] == "debug"
        else:
            msg = f"{now} Wrong arguments"
            bad_arguments = True

        # Logging the call
        with open(LOG_FILE, "a") as f:
            f.write(msg + "\n")

        if bad_arguments:
            debug(f"# Exiting: Bad arguments. Inputted: {args}")
            sys.exit(2)

        # Main function
        process_args(args)

    except Exception as e:
        debug(str(e))
        raise


def load_alert(file_path: str) -> dict:
    """Load an alert JSON file.

    Parameters
    ----------
    file_path : str
        The path to the JSON alert file.

    Returns
    -------
    dict
        The loaded JSON object as a dictionary.
    """
    try:
        with open(file_path) as alert_file:
            return json.load(alert_file)
    except FileNotFoundError:
        debug("# Alert file %s doesn't exist" % file_path)
        sys.exit(3)
    except json.decoder.JSONDecodeError as e:
        debug(f"Failed getting json_alert: {e}")
        sys.exit(4)


def process_args(args: list):
    """Process the command-line arguments.

    Parameters
    ----------
    args : list
        The command-line arguments passed to the script.
    """
    debug("# Starting")

    alert_file_location = args[1]
    api_key: str = args[2]
    hook_url: str = args[3]

    if not is_valid_url(hook_url):
        debug(f"# Hook URL argument seems to be invalid: {hook_url}")
        sys.exit(3)

    json_alert = load_alert(alert_file_location)

    debug(f"# File location: {alert_file_location}")
    debug(f"# API Key: {api_key}")
    debug(f"# Hook Url: {hook_url}")
    debug(f"# Processing alert: {json_alert}")

    maltiverse_api = Maltiverse(endpoint=hook_url, auth_token=api_key)

    # Request Maltiverse info and send event to
    # Wazuh Manager in case of positive match
    for msg in request_maltiverse_info(json_alert, maltiverse_api):
        send_event(msg, json_alert["agent"])


def debug(msg: str):
    """Print a debug message.

    Parameters
    ----------
    msg : str
        The debug message to print.
    """
    if debug_enabled:
        msg = "{0}: {1}\n".format(now, msg)
        print(msg)
        with open(LOG_FILE, "a") as f:
            f.write(msg)


def get_ioc_confidence(ioc: dict) -> str:
    """Get the vendor-neutral confidence rating and returns the None/Low/Medium/High scale.

    Parameters
    ----------
    ioc : dict
        The IOC dictionary.

    Returns
    -------
    str
        The confidence rating.
    """
    if not (classification := ioc.get("classification")):
        return "Not Specified"

    sightings = len(ioc.get("blacklist", []))
    if classification == "malicious":
        return "High" if sightings > 1 else "Medium"
    elif classification == "suspicious":
        return "Medium" if sightings > 1 else "Low"
    elif classification in ("neutral", "whitelist"):
        return "Low" if sightings > 1 else "None"


def get_mitre_information(ioc: dict) -> dict:
    """Get the MITRE information from the IOC dictionary.

    Parameters
    ----------
    ioc : dict
        The IOC dictionary.

    Returns
    -------
    dict
        The MITRE information as a dictionary.
    """
    mitre_info = {}
    for indicator in ioc.get("blacklist", []):
        for external_references in indicator.get("external_references", []):
            # filter by mitre known attacks
            if external_references.get("source_name") != "mitre-attack":
                continue

            # get the last occurrence since it should be more updated
            if external_references.get("external_id", "").startswith("S"):
                mitre_info["software"] = {
                    "id": external_references.get("external_id"),
                    "reference": external_references.get("url"),
                    "name": external_references.get("description"),
                }
    return mitre_info


def match_ecs_type(maltiverse_type: str) -> str:
    """ Convert the Maltiverse type to the ECS Threat type.

    Parameters
    ----------
    maltiverse_type : str
        The Maltiverse type.

    Returns
    -------
    str
        The ECS Threat type.
    """
    mapping = {
        "ip": "ipv4-addr",
        "hostname": "domain-name",
        "sample": "file",
        "url": "url",
    }
    return mapping.get(maltiverse_type)


def maltiverse_alert(
    alert_id: int,
    ioc_dict: dict,
    ioc_name: str,
    ioc_ref: str = None,
    include_full_source: bool = True,
) -> dict:
    """Generate a new alert.

    Parameters
    ----------
    alert_id : int
        The generated alert ID.
    ioc_dict : dict
        Raw information returned by Maltiverse API.
    ioc_name : str
        The representative name of the indicator.
    ioc_ref : str, optional
        An indicator reference used to build a reference URL.
        ioc_name is used by default if ioc_ref is not set.
    include_full_source : bool, optional (default: True)
        Whether to include the complete API response.

    Returns
    -------
    dict
        The generated alert as a dictionary.
    """
    _blacklist = ioc_dict.get("blacklist", [])
    _type = ioc_dict.get("type")
    _ref = ioc_ref if ioc_ref else ioc_name

    alert = {
        "integration": "maltiverse",
        "alert_id": alert_id,
        "maltiverse": {
            "source": ioc_dict,
        },
        "threat": {
            "indicator": {
                "name": ioc_name,
                "type": match_ecs_type(_type),
                "description": ", ".join(
                    sorted(set([b.get("description") for b in _blacklist])),
                ),
                "provider": ", ".join(
                    sorted(set([b.get("source") for b in _blacklist])),
                ),
                "first_seen": ioc_dict.get("creation_time"),
                "modified_at": ioc_dict.get("modification_time"),
                "last_seen": ioc_dict.get("modification_time"),
                "confidence": get_ioc_confidence(ioc_dict),
                "sightings": len(_blacklist),
                "reference": f"https://maltiverse.com/{_type}/{_ref}" if _type else "",
            }
        },
    }

    if _type == "ip":
        alert["threat"]["indicator"]["ip"] = ioc_name

    if (mitre_info := get_mitre_information(ioc_dict)) and "software" in mitre_info:
        alert["threat"]["software"] = mitre_info["software"]

    if not include_full_source:
        alert.pop("maltiverse")

    return alert


def request_maltiverse_info(alert: dict, maltiverse_api: Maltiverse) -> dict:
    """Request Maltiverse information and generate alerts.

    Parameters
    ----------
    alert : dict
        The alert dictionary.
    maltiverse_api : Maltiverse
        An instance of the Maltiverse class.

    Returns
    -------
    dict
        The generated alerts as a dictionary.
    """
    results = []

    if "syscheck" in alert and "md5_after" in alert["syscheck"]:
        debug("# Maltiverse: MD5 checksum present in the alert")
        md5 = alert["data"]["md5_after"]

        if md5_ioc := maltiverse_api.sample_get_by_md5(md5):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=md5_ioc,
                    ioc_name=md5,
                )
            )

    if "syscheck" in alert and "sha1_after" in alert["syscheck"]:
        debug("# Maltiverse: SHA1 checksum present in the alert")
        sha1 = alert["data"]["sha1_after"]

        if sha1_ioc := maltiverse_api.sample_get_by_sha1(sha1):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=sha1_ioc,
                    ioc_name=sha1,
                )
            )

    if "data" in alert and "srcip" in alert["data"]:
        debug("# Maltiverse: Source IP Address present in the alert")
        ipv4 = alert["data"]["srcip"]

        if not ipaddress.IPv4Address(ipv4).is_private:
            if ipv4_ioc := maltiverse_api.ip_get(ipv4):
                results.append(
                    maltiverse_alert(
                        alert_id=alert["id"],
                        ioc_dict=ipv4_ioc,
                        ioc_name=ipv4,
                    )
                )

    if "data" in alert and "hostname" in alert["data"]:
        debug("# Maltiverse: Hostname present in the alert")
        hostname = alert["data"]["hostname"]

        if hostname_ioc := maltiverse_api.hostname_get(hostname):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=hostname_ioc,
                    ioc_name=hostname,
                )
            )

    if "data" in alert and "url" in alert["data"]:
        debug("# Maltiverse: URL present in the alert")
        url = alert["data"]["url"]
        urlchecksum = hashlib.sha256(url.encode("utf-8")).hexdigest()

        if url_ioc := maltiverse_api.url_get(urlchecksum):
            results.append(
                maltiverse_alert(
                    alert_id=alert["id"],
                    ioc_dict=url_ioc,
                    ioc_name=url,
                    ioc_ref=urlchecksum,
                )
            )

    return results


def send_event(msg: str, agent: dict = None):
    """Send an event to the Wazuh Manager.

    Parameters
    ----------
    msg : str
        The event message.
    agent : dict, optional
        The agent information.
    """
    if not agent or agent["id"] == "000":
        string = f"1:maltiverse:{json.dumps(msg)}"
    else:
        location = "[{0}] ({1}) {2}".format(
            agent["id"],
            agent["name"],
            agent["ip"] if "ip" in agent else "any",
        )
        location = location.replace("|", "||").replace(":", "|:")
        string = f"1:{location}->maltiverse:{json.dumps(msg)}"

    debug(string)
    sock = socket(AF_UNIX, SOCK_DGRAM)
    sock.connect(SOCKET_ADDR)
    sock.send(string.encode())
    sock.close()


if __name__ == "__main__":
    main(sys.argv)
