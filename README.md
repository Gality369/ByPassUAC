# ByPassUAC
一些在ByPassUAC学习过程中写的小工具,后续如果有时间会上传一些自己写的BypassUAC的工具,只做学习交流使用,不打算工具化

## DisguisePEB

通过修改待伪装进程的PEB中的cmdline和imagepath来达到伪装进程的目的,这里是伪装成explorer.exe

- 用法:DisguisePEB.exe 待伪装进程名

## BypassUAC_ShellAPI_WSRest

借助win10的应用商店程序WSRest.exe,通过修改注册表中相应项下的shell/open/command实现,该进程autoElevated字段为True且相应注册表可写.

- 用法:BypassUAC_ShellAPI_WSRest.exe