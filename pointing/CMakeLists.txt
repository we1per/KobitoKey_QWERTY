if(CONFIG_KOBITOKEY_POINTING)
  target_sources(app PRIVATE src/auto_mouse.c src/precision.c)
endif()

if(CONFIG_KOBITOKEY_ALT_TAB)
  target_sources(app PRIVATE src/alt_tab.c)
endif()
