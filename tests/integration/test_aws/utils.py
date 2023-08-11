# Copyright (C) 2015-2023, Wazuh Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is free software; you can redistribute it and/or modify it under the terms of GPLv2

"""
    This file contains constant and other utilities to be used in the AWS integration test module.
"""

# CONSTANTS

ERROR_MESSAGES = {

    "failed_start": "The AWS module did not start as expected",
    "incorrect_parameters": "The AWS module was not called with the correct parameters",
    "error_found": "Found error message on AWS module",
    "incorrect_event_number": "The AWS module did not process the expected number of events"

}

TIMEOUTS = {

    10: 10,
    20: 20
}
