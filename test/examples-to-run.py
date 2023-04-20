#! /usr/bin/env python3
## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("thz-macro-central", "True", "True"),
    ("thz-nano-adhoc", "True", "True"),
    ("thz-directional-antenna", "True","True"),
    ("thz-path-loss", "True", "True"),
    ("thz-psd-macro", "True", "True"),
    ("thz-psd-nano", "True", "True"),
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
