import os

from wazuh_testing import global_parameters
from wazuh_testing.constants.paths.configurations import TEMPLATE_DIR, TEST_CASES_DIR
from wazuh_testing.utils.configuration import (
    get_test_cases_data,
    load_configuration_template,
)

TEST_DATA_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'data')


def get_test_paths(module_name):
    module_path = os.path.join(TEST_DATA_PATH, TEMPLATE_DIR, module_name)
    test_cases_path = os.path.join(TEST_DATA_PATH, TEST_CASES_DIR, module_name)
    return module_path, test_cases_path


def get_test_configuration_and_cases_paths(module_name, configuration_file, cases_file):
    module_path, test_cases_path = get_test_paths(module_name)
    configurations_path = os.path.join(module_path, configuration_file)
    cases_path = os.path.join(test_cases_path, cases_file)
    return configurations_path, cases_path