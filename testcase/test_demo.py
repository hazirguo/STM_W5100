 #-*- coding: utf-8 -*-
import socket
import sys
import time
import random

# 模拟 coral 主服务器的行为
# 每隔一段随机的时间 disable 一次互锁设备，再过一段时间 enable

HOST = "1.0.0.223"
PORT = 2101

NCD_ON_BASE = 8
NCD_OFF_BASE = 0
NCD_RELAY_SENSE_BASE = 16

CMD_HEADER = chr(0xFE)

ENABLE_CHANNEL0_CMD = CMD_HEADER + chr(NCD_ON_BASE + 0)
DISABLE_CHANNEL0_CMD = CMD_HEADER + chr(NCD_OFF_BASE + 0)
SENSE_CHANNEL0_CMD = CMD_HEADER + chr(NCD_RELAY_SENSE_BASE + 0)

ENABLE_CHANNEL1_CMD = CMD_HEADER + chr(NCD_ON_BASE + 1)
DISABLE_CHANNEL1_CMD = CMD_HEADER + chr(NCD_OFF_BASE + 1)
SENSE_CHANNEL1_CMD = CMD_HEADER + chr(NCD_RELAY_SENSE_BASE + 1)

testNum = 0

while(1):
	testNum = testNum + 1

	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	except socket.error, msg:
		print 'fail to create socket, error code: ' + str(msg[0]) + ', error msg: ' + msg[1]
		sys.exit()

## 每条命令有以下几个动作：
		# connect
		# send cmd
		# recv
		# send cmd_sense
		# recv
		# close

	print '####################test %d start#########################' % testNum
	
	# 发送 disable 命令给channel0
	s.connect((HOST, PORT))

	s.send(DISABLE_CHANNEL0_CMD)
	# 接收命令的返回码
	ret = s.recv(1024)
	if ord(ret) != 0x55:
		print 'return code error'
		s.close()
		sys.exit()

	s.send(SENSE_CHANNEL0_CMD)
	ret = s.recv(1024)
	if ord(ret) != 0:
		print 'disable failed'
		s.close()
		sys.exit()

	print 'disable channel0 successed!'
	s.close()

	time.sleep(1)

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))
	# 发送 disable 命令给channel1
	s.send(DISABLE_CHANNEL1_CMD)
	# 接收命令的返回码
	ret = s.recv(1024)
	if ord(ret) != 0x55:
		print 'return code error'
		s.close()
		sys.exit()

	s.send(SENSE_CHANNEL1_CMD)
	ret = s.recv(1024)
	if ord(ret) != 0:
		print 'disable failed'
		s.close()
		sys.exit()

	print 'disable channel1 successed!'
	s.close()

	time.sleep(10)   # 10s 之后打开

	#########################
	# 发送 enable 命令给channel0
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))

	s.send(ENABLE_CHANNEL0_CMD)
	#接收命令的返回码
	ret = s.recv(1024)
	if ord(ret) != 0x55:
		print 'return code error'
		s.close()
		sys.exit()

	s.send(SENSE_CHANNEL0_CMD)
	ret = s.recv(1024)
	if ord(ret) != 1:
		print 'enable failed'
		s.close()
		sys.exit()

	print 'enable channel0 successed!'
	s.close()

	time.sleep(1)

	# 发送 enable 命令给channel1
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))

	s.send(ENABLE_CHANNEL1_CMD)
	# 接收命令的返回码 
	ret = s.recv(1024)
	if ord(ret) != 0x55:
		print 'return code error'
		s.close()
		sys.exit()

	s.send(SENSE_CHANNEL1_CMD)
	ret = s.recv(1024)
	if ord(ret) != 1:
		print 'enable failed'
		s.close()
		sys.exit()

	print 'enable channel1 successed!'
	s.close()
	
	print '####################test %d end#########################' % testNum
	
	sleepTime = random.randint(10, 1000)
	print 'sleep for %d seconds\n' % sleepTime
	time.sleep(sleepTime)
	
	
	

