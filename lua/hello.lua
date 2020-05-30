#! /usr/local/bin/lua
-- this is a annotation
--[[
print("annotation")
this is a annotation

--]]
print(_Version)
print("hello world");
print("hello wms")

name = "wms"
print(name)

print(type("Hello world"))      --> string
print(type(10.4*3))             --> number
print(type(print))              --> function
print(type(type))               --> function
print(type(true))               --> boolean
print(type(nil))                --> nil
print(type(type(X)))            --> string

