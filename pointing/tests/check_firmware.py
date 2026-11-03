"""Check the linked event order required to cancel before resolving the first key.

Usage: python pointing/tests/check_firmware.py path/to/build/zephyr/zmk.elf
Requires pyelftools (included in Zephyr's Python requirements).
"""
import sys
from elftools.elf.elffile import ELFFile

with open(sys.argv[1], "rb") as stream:
    elf = ELFFile(stream)
    symbols = {symbol.name: symbol["st_value"] for symbol in elf.get_section_by_name(".symtab").iter_symbols()}
    auto = symbols["zmk_event_sub_kobito_auto_mousezmk_position_state_changed"]
    keymap = symbols["zmk_event_sub_keymapzmk_position_state_changed"]
    if auto >= keymap:
        raise SystemExit("FAIL: auto mouse must receive a press before keymap (including &none)")
    alt_tab = symbols["zmk_event_sub_kobito_alt_tabzmk_keycode_state_changed"]
    hid = symbols["zmk_event_sub_hid_listenerzmk_keycode_state_changed"]
    if alt_tab >= hid:
        raise SystemExit("FAIL: Alt-Tab must consume navigation keys before HID")
    for name in ("motion_timeout", "click_timeout", "prior_idle", "auto_enabled", "precision_divisor"):
        if name not in symbols:
            raise SystemExit(f"FAIL: missing Studio setting {name}")
    print("PASS: auto-mouse listener precedes keymap; five custom settings linked")
    print("PASS: Alt-Tab listener precedes HID")
