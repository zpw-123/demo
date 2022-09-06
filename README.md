# demo
选择 quartz模拟NVM


第一步
First, load the emulator's kernel module. From the emulator's source code root folder, execute: 

sudo scripts/setupdev.sh load

第二步
Set your processor to run at maximum frequency to ensure fixed cycle rate (as the cycle counter is used to project delay time). You can use the scaling governor:

echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

第三步
Set the LD_PRELOAD and NVMEMUL_INI environment variables to point respectively to the emulators library and the configuration file to be used. The LD_PRELOAD is used for automatically loading the emulator's library when the user application is executed. Thus, there is no need to statically link the library to the user application. See below details about the configuration file in the respective section.

Rather than configuring the scaling governor and the environment variables manually as indicated above, you can use the scripts/runenv.sh script. See below.

An additional configuration step may be required depending on the Linux Kernel version. This emulator makes use of rdpmc x86 instruction to read CPU counters. Before kernel 4.0, when rdpmc support was enabled, any process (not just ones with an active perf event) could use the rdpmc instruction to access the counters. Starting with Linux 4.0 rdpmc support is only allowed if an event is currently enabled in a process's context. To restore the old behavior, write the value 2 to /sys/devices/cpu/rdpmc if kernel version is 4.0 or greater:

echo 2 | sudo tee /sys/devices/cpu/rdpmc

第四步
Run your application:

scripts/runenv.sh <your_app>
其中  <your_app>是  通过 指令  g++ /home/zpw/HME-Quartz-broadwell-master/ndgraph/update.cpp -fopenmp 生成的a.out文件

第五步：
sudo ./a.out <数据集>
即可运行出来所需结果

以上运行完成插入边的过程

ps1:
quartz模拟的时间跟 不模拟运行时间大致一样 (不模拟 即可跳过前三步 从第四步直接g++编译 然后第五步即可)
ps2:
关于数据集 我修改了师兄代码 边表形式数据集即可读取 


