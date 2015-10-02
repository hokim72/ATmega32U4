cmake_minimum_required(VERSION 3.1)


# MCU name
set(MCU atmega32u4)

# Target architecture (see library "Board Types" documentaion).
set(ARCH AVR8)

# Target board (see library "Board Types" documentation, NONE for projects not requiring
# LUFA board drivers). If USER is selected, put custom board drivers in a directory called
# "Board" inside the application directory.
set(BOARD LEONARDO)

# Process frequency.
#	This will define a symbol, F_CPU, in all source code files equal to the
#	processor frequency in Hz. You can then use this symbol in your source code to
#	calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#	automatically to create a 32-bit value in your source code.

#	This will be an integer division of F_USB below, as it is sourced by
#	F_USB after it has run through any CPU prescalers. Note that this value
#	does not *change* the processor frequency - it should merely be updated to
#	reflect the processor speed set externally so that the code can use accurate
#	software delays.
set(F_CPU 16000000)

# Input clock frequency.
#	This will define a symbol, F_USB, in all source code files equal to the
#	input clock frequency (before any prescaling is performed) in Hz. This value may
#	differ from F_CPU if prescaling is used on the latter, and is required as the
#	raw input clock is fed directly to the PLL sections of the AVR for high speed
#	clock genertion for the USB and other AVR subsections. Do NOT tack on a 'UL'
#	at the end, this will be done automatically to create a 32-bit value in your
#	source code.

#	If no clock division is performed on the input clock inside the AVR (via the
#	CPU clock adjust registers or the clock division fuses), this will be equal to F_CPU.
set(F_USB ${F_CPU})

# Target file name (without extension).
set(TARGET GenericHID)

# Path to the LUFA library
set(LUFA_PATH $ENV{AVR_COMMON}/lufa-LUFA-140928)

set(AVRLIB $ENV{AVR_COMMON}/avrlib)

# LUFA library compile-time options and predefined tokens
set(LUFA_OPTS
	-D USE_STATIC_OPTIONS="\(USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL\)"
	-D USB_DEVICE_ONLY
	-D USE_FLASH_DESCRIPTORS
	-D FIXED_CONTROL_ENDPOINT_SIZE=8
	-D FIXED_NUM_CONFIGURATIONS=1
	-D GENERIC_REPORT_SIZE=8
)	
string(REPLACE ";" " " LUFA_OPTS "${LUFA_OPTS}")

# Create the LUFA source path varaibles by including the LUFA root cmake file
include($ENV{AVR_COMMON}/lufa140928.cmake)

set(AVRLIB_SRCS
	${AVRLIB}/buffer.c
	${AVRLIB}/rprintf.c
	${AVRLIB}/uart.c
)
# List C source files here. (C dependencies are automatically generated.)
set(SRCS ${TARGET}.c Descriptors.c ${LUFA_SRC_USB} ${LUFA_SRC_USBCLASS} ${AVRLIB_SRCS})

# Optimization level, can be [0, 1, 2, 3, s].
#	0 = turn off optimization. s = optimize for size.
#	(Note: 3 is not always the best optimization level. See avr-libc FAQ.)
set(OPT s)

# Debugging format
#	Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#	AVR Studio 4.10 requires dwarf-2.
#	AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
set(DEBUG dwarf-2)


# Compiler flag to set the C Standard level.
# 	C89 = "ANSI" C
#	gnu89 = c89 plus GCC extensions
#	c99 = ISO C99 standard (not yet fully implemented)
#	gnu99 = c99 plus GCC extensions
set(CSTANDARD -std=c99)

# Place -D or -U options here for C sources
set(CPP_FLAGS 
	-DF_CPU=${F_CPU}UL
	-DF_USB=${F_USB}UL
	-DBOARD=BOARD_${BOARD} -DARCH=ARCH_${ARCH}
	${LUFA_OPTS}
)
string(REPLACE ";" " " CPP_FLAGS "${CPP_FLAGS}")

#---------------- Compiler Options C ----------------
#	-g*: generate debugging information
#	-O*: optimization level
#	-f...: tuning, see GCC manual and avr-libc documentation
#	-Wall..: warning level
#	-Wa,...: tell GCC to pass this to assembler.
#	-adhlns...: create assembler listing
set(C_FLAGS 
	-mmcu=${MCU}
	-g${DEBUG} 
	-O${OPT} 
	-fshort-enums
	-fno-inline-small-functions 
	-fpack-struct 
	-fno-strict-aliasing
	-funsigned-char 
	-funsigned-bitfields 
	-ffunction-sections 
	-Wall
	-Wstrict-prototypes
	#-mshort-calls
	#-fno-unit-at-a-time
	#-Wundef
	#-Wunreachable-code
	#-Wsign-compare
	-Wa,-adhlns=\"$@.lst\"
	${CSTANDARD}
	-MMD -MP
)
string(REPLACE ";" " " C_FLAGS "${C_FLAGS}")
include_directories(${CMAKE_SOURCE_DIR} ${LUFA_PATH} ${AVRLIB})

set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")

#---------------- Linker Options ----------------
# -Wl,...: tell GCC to pass this to linker.
# -Map: create map file
# --cref: add cross reference to map file
set(LD_FLAGS 
	-mmcu=${MCU}
	"-Wl,-Map=${TARGET}.map,--cref"
	"-Wl,--relax"
	"-Wl,--gc-sections"
)
string(REPLACE ";" " " LD_FLAGS "${LD_FLAGS}")


add_executable(${TARGET}.elf ${SRCS})
#set_target_properties(${TARGET}.elf PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")


add_custom_target(${TARGET} ALL
    COMMAND ${OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.hex
    COMMAND ${OBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings ${TARGET}.elf ${TARGET}.eep
    COMMAND ${OBJCOPY} -O binary -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.bin
    COMMAND ${OBJDUMP} -h -d -S -z ${TARGET}.elf > ${TARGET}.lss
    COMMAND ${NM} -n ${TARGET}.elf > ${TARGET}.sym
    COMMAND ${SIZE} -C --mcu=${MCU} ${TARGET}.elf
    DEPENDS ${TARGET}.elf
)

set(PROG_TYPE avr109)
set(PROG_ARGS -P /dev/ttyACM0 -b 57600)

add_custom_target(flash
    COMMAND ${AVRDUDE}  -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -e -U flash:w:${TARGET}.hex
    DEPENDS ${TARGET}
)

