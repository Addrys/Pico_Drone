cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME mpu6050_i2c)
set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)


project(mpu6050_i2c)

pico_sdk_init()


if (TARGET tinyusb_device)
	add_executable(mpu6050_i2c
			main.cpp
            mpu.cpp  
            pid.cpp
			)

    target_include_directories(${PROGRAM_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
	# pull in common dependencies and additional i2c hardware support
	target_link_libraries(mpu6050_i2c 
        pico_stdlib 
        hardware_i2c
        hardware_pio
        hardware_timer
        hardware_clocks
        pico_cyw43_arch_none
    )

	# create map/bin/hex file etc.
	pico_add_extra_outputs(mpu6050_i2c)
    
	# add url via pico_set_program_url
	#example_auto_set_url(mpu6050_i2c)

	
    # enable usb output, disable uart output
    pico_enable_stdio_usb(mpu6050_i2c 1)
    pico_enable_stdio_uart(mpu6050_i2c 0)


elseif(PICO_ON_DEVICE)
    message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()