# encoding:utf-8
import os
from ctypes import *

if(os.path.exists('stm32.dylib')):
	stm32 = CDLL('stm32.dylib')
else:
	stm32 = CDLL('com/stm32.dylib')

def get_pc_version():
	func = stm32.get_pc_version
	#func.argtypes = ()
	func.restype = c_char_p
	return func()

def get_usb_vendor():
	func = stm32.get_usb_vendor
	func.restype = c_char_p
	return func()

def get_usb_product():
	func = stm32.get_usb_product
	func.restype = c_char_p
	return func()

def open_usb():
	return stm32.open_usb()

def close_usb():
	stm32.close_usb()

def enter_usb():
	return stm32.enter_usb()

def leave_usb():
	return stm32.leave_usb()

def set_usb_vid(vid):
	return stm32.set_usb_vid(vid)

def set_usb_pid(pid):
	return stm32.set_usb_vid(pid)

def ping():
	return stm32.ping()

def get_firmware():
	func = stm32.get_firmware
	func.restype = c_char_p
	return func()

def soft_reset():
	return stm32.soft_reset()

def set_gpio_out(gpio_type, gpio_pin):
	return stm32.set_gpio_out(gpio_type, gpio_pin)

def set_gpio_in(gpio_type, gpio_pin):
	return stm32.set_gpio_in(gpio_type, gpio_pin)

def set_gpio_high(gpio_type, gpio_pin):
	return stm32.set_gpio_high(gpio_type, gpio_pin)

def set_gpio_low(gpio_type, gpio_pin):
	return stm32.set_gpio_low(gpio_type, gpio_pin)

def read_gpio(gpio_type, gpio_pin):
	status = c_byte(0)
	stm32.read_gpio(gpio_type, gpio_pin, byref(status))
	return status.value

def config_check():
	return stm32.config_check()

def config_clear():
	return stm32.config_clear()

def config_get_content(id):
	size = c_ulong(512)
	buf  = create_string_buffer(size.value)
	if 0==stm32.config_get_content(id,buf,byref(size)):
		return [ord(buf.raw[i]) for i in range(0,size.value)]
	else:
		return []

def config_update_content(id, li):
	str  = "".join(chr(0xFF&i) for i in li)
	size = len(str)
	return stm32.config_update_content(id, create_string_buffer(str, size), size)

def config_get_content_string(id):
	li = config_get_content(id)
	return "".join(chr(0xFF&i) for i in li)

def config_update_content_string(id, str):
	return config_update_content(id, [ord(i) for i in str])

def mpu6050_read_reg(addr, len):
	buf = create_string_buffer(len)
	if 0==stm32.mpu6050_read_reg(addr,buf,len):
		return [ord(buf.raw[i]) for i in range(0,len)]
	else:
		return []

def mpu6050_write_reg(addr, li):
	str  = "".join(chr(0xFF&i) for i in li)
	size = len(str)
	return stm32.mpu6050_write_reg(addr, create_string_buffer(str, size), size)
