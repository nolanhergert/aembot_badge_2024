all : flash

CH32V003FUN:=ch32v003fun/ch32v003fun
TARGET:=tim1_pwm
# ADDITIONAL_C_FILES:=patterns.c

include ch32v003fun/ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

