# deviceQuery
Nodejs Native AddOn to query hardware info

	基于WMI获取设备属性：
		0：网卡原生MAC地址
		1：硬盘序列号
		2：主板序列号
		3：CPU ID
		4：BIOS序列号
		5：主板型号
		6：网卡当前MAC地址

实现Nodejs调用C/C++
 >Node.js 官方提供了C/C++ Addons 的机制让我们能够使用 V8 API 把 Node.js 和 C++ 结合起来
 
为什么要在Nodejs调用C/C++

如何在Nodejs中调用动态链接库
 >ffi  swig
