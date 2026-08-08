_debug = False
_print_msg = True

_disable_all_warnings = False
_disable_duplicated_warnings = True
_disabled_warning = set()
_disable_warnings_below_this_level = 0
_warning_record = {}


def debug_log(*args, **kwargs):
    if _debug:
        print('[DEBUG] ', end='')
        print(*args, **kwargs)


def warn_log(warn_level: int, warn_type: str, *args, **kwargs):
    # warn_level ranges from 1 to inf; larger levels are more severe.
    if _disable_all_warnings:
        return
    if warn_type in _disabled_warning:
        return
    if warn_level < _disable_warnings_below_this_level:
        return
    triggered_count = _warning_record.get(warn_type, 0)
    if _disable_duplicated_warnings and triggered_count > 0:
        return
    _warning_record[warn_type] = triggered_count + 1
    print(f'[WARNING {warn_level}: {warn_type}] ', end='')
    print(*args, **kwargs)


def msg_log(*args, **kwargs):
    if _print_msg:
        print('[MESSAGE] ', end='')
        print(*args, **kwargs)


def warn_signal_contains_current_input(signal_name):
    warn_log(
        1,
        f"signal-contains-current-input:{signal_name}",
        f"expr(dut.{signal_name}.value) contains current inputvars; "
        f"modifying related inputvars afterward may cause dut.{signal_name}.value to change."
    )


def clear_warning_record():
    _warning_record.clear()
