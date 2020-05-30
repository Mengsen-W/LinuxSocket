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

-- 使用两个方括号来表示一块字符
html = [[
<html>
<head></head>
<body>
    <a href="http://https::///">test</a>
    </body>
    </html>
]]
print(html)

print("2".."8")
print('2'+'8')
print('2'+8)
-- must blank with '..' whit number
print(125 .. 24)

print(#html)
