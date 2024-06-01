set(SRC_DIRS Src)
set(INCLUDE_DIRS Inc)

file(WRITE ${CMAKE_SOURCE_DIR}/check_button_int.c "
#include \"${CMAKE_SOURCE_DIR}/wifiManager_private.h\"
int main() {
#ifdef USE_BUTTON_INT
    return FEATURE_ENABLE;
#else
    return 0;
#endif
}
")

try_compile(HAS_BTN_INT_ENABLED
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/check_feature.c
)

if(HAS_BTN_INT_ENABLED)
    try_run(RUN_RESULT
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/check_feature
    )
    if(RUN_RESULT EQUAL 1)
        set(HAS_BTN_INT_ENABLED 1)
    else()
        set(HAS_BTN_INT_ENABLED 0)
    endif()
else()
    set(HAS_BTN_INT_ENABLED 0)
endif()

if(HAS_BTN_INT_ENABLED)
    list(APPEND SRC_DIRS Src/Feature)
    list(APPEND INCLUDE_DIRS Inc/Feature)
    set(REQUIRES nvs_flash esp_http_server esp_wifi esp_netif esp_event esp_eth lwip efuse driver)
else()
    set(REQUIRES nvs_flash esp_http_server esp_wifi esp_netif esp_event esp_eth lwip efuse)
endif()

idf_component_register(SRC_DIRS ${SRC_DIRS}
                    	INCLUDE_DIRS ${INCLUDE_DIRS}
                    	REQUIRES ${REQUIRES}
											EMBED_FILES Web/index.html Web/scan.css Web/app.js Web/password.html Web/password.css Web/password.js 
															Web/lib/jquery-3.3.1.min.js Web/icons/favicon.ico Web/icons/wifi_full.ico Web/icons/wifi_three.ico 
															Web/icons/wifi_two.ico Web/icons/wifi_err.ico)

file(REMOVE ${CMAKE_SOURCE_DIR}/check_feature.c)