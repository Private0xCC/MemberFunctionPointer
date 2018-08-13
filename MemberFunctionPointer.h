#pragma once

//代码用到了不定长模板参数，编译器需要支持 C++11 标准
#if __cplusplus < 199711L
#error [Private.MemberFunctionPointer]需要编译器支持 C++11 标准
#endif

/*
* 在vc环境下,影响成员函数指针的表示形式的方式有三种
* 1.编译器开关 /vmg /vmb
* https://msdn.microsoft.com/zh-cn/library/yad46a6z.aspx
* 2.#pragma pointers_to_members( pointer-declaration, [most-general-representation] ) [C++ 专用]
* https://msdn.microsoft.com/zh-cn/library/83cch5a6.aspx
* 3.继承关键字 __single_inheritance,__multiple_inheritance,__virtual_inheritance [Microsoft 专用]
* https://msdn.microsoft.com/zh-cn/library/ck561bfk.aspx
* 三种方式，优先级递增
*/

/*
* 所以,在vc环境下,成员函数指针的表示形式一共有四种
* 1.单继承类的成员函数指针,包含 1 个数据:函数的实际地址(对于虚函数来说则是 vcall thunk(virtual call thunk,翻译过来就是 虚拟调用转换 ). 具体参考 【关于 vcall thunk】
* 2.多继承类的成员函数指针,包含 2 个数据: 函数的实际地址,this 调整值(有关this调整值具体可以参考多继承的内存布局)
* 3.虚继承类的成员函数指针,包含 3 个数据: 函数的实际地址,this 调整值,虚基类表索引(不是虚函数表 vbptr != vfptr),具体参考下面的代码实现和注释
* 4.完整的成员函数指针,包含 3 个数据: 函数的实际地址,this 调整值,虚基类表偏移量(从 this 到 虚基类表),虚基类表索引
* 注意 虚继承类的成员函数指针 与 完整的成员函数指针 之间存在的差异, 虚继承类的成员函数指针第 3 个数据是 [虚基类表索引],而完整的成员函数指针第 4 个数据才是
* 具体的指针结构组成请参考 成员函数指针结构定义
*/

/* 关于 vcall thunk
* 实际上vcall thunk 就是实际函数的跳转函数,函数内部会跳转到虚函数表中的slot位置对应的函数地址.
* 微软使用 vcall thunk 技术来支持虚函数,所有的同名的虚函数(父类的虚函数和子类重写后的虚函数)都对应同一个vcall,
* 之所以同名的虚函数可以和vcall thunk 多对一,是因为,不管是父类还是子类,同名的虚函数在虚函数表中的位置是一致的)
* 举个例子来说就是,父类中的虚函数 A 在父类的虚函数表中是第 2 slot,那么子类重写 A 函数后,虚函数 A 在子类的虚函数表中
* 是也第 2 slot,因为 子类的虚函数表自父类继承而来,子类只会拓展虚函数表(子类可以有自己的虚函数),
* 不会改变继承过来的虚函数表,有点类似类的继承.
* 每个 vcall thunk 对应的 slot 是固定的,然后会根据 this (或者调整后的this)找到虚函数表,
* 计算方式为:const char * vbptr = *(char**)pthis , 计算原理就是 类的第一个成员就是 vfptr(如果存在虚函数)
* 最后在虚函数表中的slot位置获取到实际的函数地址进行跳转
*/

#pragma region 静态断言宏
#define __STATIC_ASSERT_MFP_SIZE_CHECK__ \
static_assert(sizeof(void(__SIC::*)()) == sizeof(Private::SI_MFP), "An unexpected mistake,[single inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__MIC::*)()) == sizeof(Private::MI_MFP), "An unexpected mistake,[multiple inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__VIC::*)()) == sizeof(Private::VI_MFP), "An unexpected mistake,[virtual inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__UND::*)()) == sizeof(Private::Full_MFP), "An unexpected mistake,[Full member function pointer]  size is not consistent with expectations.");

#define __STATIC_ASSERT_SI_MFP_CHECK__(mfp) \
static_assert(sizeof(mfp) == (unsigned char)MFP_TYPE::SI_MFP, "[mfp] Not a single inherited member function pointer")
#define __STATIC_ASSERT_MI_MFP_CHECK__(mfp) \
static_assert(sizeof(mfp) == (unsigned char)MFP_TYPE::MI_MFP, "[mfp] Not a multiple inherited member function pointer")
#define __STATIC_ASSERT_VI_MFP_CHECK__(mfp) \
static_assert(sizeof(mfp) == (unsigned char)MFP_TYPE::VI_MFP, "[mfp] Not a virtual inherited member function pointer")
#define __STATIC_ASSERT_FULL_MFP_CHECK__(mfp) \
static_assert(sizeof(mfp) == (unsigned char)MFP_TYPE::Full_MFP, "[mfp] Not a Full member function pointer")
#define __STATIC_ASSERT_MFP_CHECK__(mfp) \
static_assert(sizeof(mfp) <= (unsigned char)MFP_TYPE::Full_MFP, "[mfp] Not a valid member function pointer")

#pragma endregion

namespace Private
{

#pragma region  SomeClass

	class __Base {};
	//无继承类
	class __NIC {};
	//单继承类
	class __single_inheritance __SIC : __Base{};
	//多继承类
	class __multiple_inheritance __MIC : __Base, __NIC{};
	//虚继承类
	class __virtual_inheritance __VIC : virtual __Base{};

	//未定义的类,在定义类之前来声明指向类成员的指针,则该类的成员指针是一个完整的指针。
	//因为在定义类之前，编译器不知道类的继承方式，不知道该类成员指针的最佳表示方式，
	//所以就以完整的成员函数指针来表示，以兼容未知的所有情况
	//可以参考 https://msdn.microsoft.com/zh-cn/library/yad46a6z.aspx
	class __UND;

#pragma endregion

#pragma region  成员函数指针结构定义
	//单继承成员函数指针
	class SI_MFP
	{
	protected:
		//只包含实际函数地址
		void * CodePtr;

		void * GetThis();
	public:
		SI_MFP();

		template<class T, class T_Ret, class ...T_Args>
		SI_MFP(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		SI_MFP & operator=(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		SI_MFP & Deassign(T_Ret(T::*mfp)(T_Args...));

		void * GetCodePtr() const;

		SI_MFP & SetCodePtr(void * codeptr);

		template<class T, class T_Ret, class ...T_Args>
		SI_MFP & SetCodePtr(T_Ret(T::*mfp)(T_Args...));

		//调用成员函数指针，需要保证返回值与参数列表与实际函数签名一致，否则可能会导致调用异常
		template<class T_Ret, class ... T_Args>
		T_Ret Apply(void * pthis, T_Args ... args) const;

		//事实上，我们不需要关心编译器是如何完成寻址的
		//我们只需要知道，通过成员函数指针进行函数调用的时候
		//编译器会自动完成寻址
		//也就是说，在调用实际的成员函数时，this 指针是已经进行过调整的
		//所以，我们的寻址操作原理是：修改 原始的成员函数指针 的 [实际函数地址]
		//这个函数地址指向 我们自定义的一个函数，这个函数只有一个简单的 return this;
		//由于只修改了 [实际函数地址]，成员函数指针的其他数据没有修改
		//所以编译器的寻址结果与修改前的寻址结果是一样的
		//[实际函数地址] 指向的函数可以将 编译器寻址后的结果 this 返回。
		//我们不需要关心成员函数指针的组成，也不需要关心如何寻址，只需要保证this是正确即可
		//这一切编译器会帮我们完成
		void * Addressing(void * pthis) const;

		void * AutoAddressing(void * pthis) const;

		//转换为C++原生的成员函数指针，请注意返回值与参数列表
		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const SI_MFP & that) const;
	};

	//多继承成员函数指针
	class MI_MFP :public SI_MFP
	{
	protected:
		//在单继承成员函数指针基础上多了 this 调整值
		int Delta;
	public:
		MI_MFP();

		template<class T, class T_Ret, class ...T_Args>
		MI_MFP(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		MI_MFP & operator=(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		MI_MFP & Deassign(T_Ret(T::*mfp)(T_Args...));

		int GetDelta() const;
		MI_MFP & SetDelta(int delta);

		template<class T_Ret, class ... T_Args>
		T_Ret Apply(void * pthis, T_Args ... args) const;

		void * Addressing(void * pthis) const;

		void * AutoAddressing(void * pthis) const;

		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const MI_MFP & that) const;
	};

	//虚继承成员函数指针
	class VI_MFP :public MI_MFP
	{
	protected:
		//在多继承成员函数指针继承上多了虚基类表索引
		//该值指示了应该在[虚基类表]中哪个位置(以字节为单位)开始取值(取int)得到[虚基类]的偏移量
		int VBTableIndex;
	public:
		VI_MFP();

		template<class T, class T_Ret, class ...T_Args>
		VI_MFP(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		VI_MFP & operator=(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		VI_MFP & Deassign(T_Ret(T::*mfp)(T_Args...));

		int GetVBTableIndex() const;

		VI_MFP & SetVBTableIndex(int vbtable_index);

		template<class T_Ret, class T, class ... T_Args>
		T_Ret Apply(T * pthis, T_Args ... args) const;

		//寻址出来可能是基类子对象，所以不 返回 T*
		template<class T>
		void * Addressing(T * pthis) const;

		//寻址出来可能是基类子对象，所以不 返回 T*
		template<class T>
		void * AutoAddressing(T * pthis) const;

		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const VI_MFP & that) const;

	};

	//完整的成员函数指针
	//之所以不直接继承 VI_MFP ,是因为 完整的成员函数指针 第四个成员才是 VBTableIndex
	class Full_MFP :public MI_MFP
	{
	protected:
		//this 到 vbptr 的偏移量,并不是 this(子类this) 到基类子对象的偏移量
		int FVtorDisp;
		//虚基类表索引
		int VBTableIndex;
	public:
		Full_MFP();

		template<class T, class T_Ret, class ...T_Args>
		Full_MFP(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		Full_MFP & operator=(T_Ret(T::*mfp)(T_Args...));

		template<class T, class T_Ret, class ...T_Args>
		Full_MFP & Deassign(T_Ret(T::*mfp)(T_Args...));

		template<class T_Ret, class ... T_Args>
		T_Ret Apply(void * pthis, T_Args ... args) const;

		void * Addressing(void * pthis) const;

		void * AutoAddressing(void * pthis) const;

		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const Full_MFP & that) const;

#pragma region Getter/Setter

		int GetFVtorDisp() const;

		Full_MFP & SetFVtorDisp(int fVtorDisp);

		int GetVBTableIndex() const;

		Full_MFP & SetVBTableIndex(int vbtable_index);
#pragma endregion
	};

#pragma endregion

	//以下断言可以确保实际的成员函数指针大小与我们期望的一致
	//如果不一致，则说明不是VS环境，或者微软编译器对成员函数指针的实现方式发生了改变(这几乎不可能)
	//未编译时,下面的静态断言可能会提示失败,不用担心,如果你有强迫症,可以注释掉.我就有强迫症.
	__STATIC_ASSERT_MFP_SIZE_CHECK__;

	//成员函数指针类型的枚举值
	enum class MFP_TYPE :unsigned char
	{
		//无效成员函数指针
		Invalid = 0,
		//单继承类的成员函数指针
		SI_MFP = sizeof(void(__SIC::*)()),
		//多继承类的成员函数指针
		MI_MFP = sizeof(void(__MIC::*)()),
		//虚继承类的成员函数指针
		VI_MFP = sizeof(void(__VIC::*)()),
		//完整成员函数指针
		Full_MFP = sizeof(void(__UND::*)()),
	};

#pragma region 成员函数指针结构 实现

#pragma region 单继承成员函数指针

	SI_MFP & SI_MFP::SetCodePtr(void * codeptr)
	{
		this->CodePtr = codeptr;
		return *this;
	}

	void * SI_MFP::GetThis()
	{
		return this;
	}

	SI_MFP::SI_MFP() :CodePtr(nullptr)
	{
	}

	template<class T, class T_Ret, class ...T_Args>
	SI_MFP::SI_MFP(T_Ret(T::*mfp)(T_Args...))
	{
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	SI_MFP & SI_MFP::operator=(T_Ret(T::*mfp)(T_Args...))
	{
		return this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	SI_MFP & SI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_SI_MFP_CHECK__(mfp);
		void * p_mfp = &mfp;
		this->CodePtr = *(void **)p_mfp;
		return *this;
	}

	void * SI_MFP::GetCodePtr() const
	{
		return CodePtr;
	}

	template<class T, class T_Ret, class ...T_Args>
	SI_MFP & SI_MFP::SetCodePtr(T_Ret(T::*mfp)(T_Args...))
	{
		//不管传入的成员函数指针是是哪种类型，我们只要CodePtr,也就是成员函数指针的第一部分
		//成员函数指针的第一部分数据，也就是函数实际地址
		void * p_mfp = &mfp;
		this->CodePtr = *(void **)p_mfp;
		return *this;
	}

	template<class T_Ret, class ... T_Args>
	T_Ret SI_MFP::Apply(void * pthis, T_Args ... args) const
	{
		//调用成员函数指针，需要调用对象，即 this 指针
		//我们不需要关心 this 的具体类型，只需要将其他任意类的成员函数指针
		//转成中间类 __SIC 的成员函数指针，然后将传入的pthis转出当前类指针，最后通过当前类指针调用即可
		//因为 __SIC 声明时指定了 __single_inheritance,，所以 __SIC 的大小与其他类的成员函数指针大小相同
		//转换时不会丢失数据

		typedef T_Ret(__SIC::*MFP)(T_Args...);

		MFP mfp;
		
		this->Convert(&mfp);

		return ((__SIC *)pthis->*mfp)(args...);
	}

	//事实上，我们不需要关心编译器是如何完成寻址的
	//我们只需要知道，通过成员函数指针进行函数调用的时候
	//编译器会自动完成寻址
	//也就是说，在调用实际的成员函数时，this 指针是已经进行过调整的
	//所以，我们的寻址操作原理是：修改 原始的成员函数指针 的 [实际函数地址]
	//这个函数地址指向 我们自定义的一个函数，这个函数只有一个简单的 return this;
	//由于只修改了 [实际函数地址]，成员函数指针的其他数据没有修改
	//所以编译器的寻址结果与修改前的寻址结果是一样的
	//[实际函数地址] 指向的函数可以将 编译器寻址后的结果 this 返回。
	//我们不需要关心成员函数指针的组成，也不需要关心如何寻址，只需要保证this是正确即可
	//这一切编译器会帮我们完成
	void * SI_MFP::Addressing(void * pthis) const
	{
		return pthis;//单继承函数指针this不需要做调整
	}

	void * SI_MFP::AutoAddressing(void * pthis) const
	{
		SI_MFP temp = *this;
		temp.SetCodePtr(&SI_MFP::GetThis);

		//寻址拿到调整后的this指针
		void* AdjustedThis = temp.Apply<void *>(pthis);

		return AdjustedThis;
	}

	bool SI_MFP::Equal(const SI_MFP & that) const
	{
		if (nullptr == &that)
		{
			return false;
		}
		return that.CodePtr == this->CodePtr;
	}

	template<class T, class T_Ret, class ... T_Args>
	bool SI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...)) const
	{
		if (sizeof(*p_mfp) == sizeof(SI_MFP))
		{
			*(void **)p_mfp = this->CodePtr;

			return true;
		}
		return false;
	}

#pragma endregion

#pragma region 多继承成员函数指针

	MI_MFP::MI_MFP() :SI_MFP(), Delta(0)
	{
	}

	template<class T, class T_Ret, class ...T_Args>
	MI_MFP::MI_MFP(T_Ret(T::*mfp)(T_Args...))
	{
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	MI_MFP & MI_MFP::operator=(T_Ret(T::*mfp)(T_Args...))
	{
		return this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	MI_MFP & MI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_MI_MFP_CHECK__(mfp);

		void * p_mfp = &mfp;
		this->CodePtr = *(void **)p_mfp;
		this->Delta = *(int *)((char *)p_mfp + sizeof(void *));
		return *this;

		return *this;
	}

	int MI_MFP::GetDelta() const
	{
		return Delta;
	}

	MI_MFP & MI_MFP::SetDelta(int delta)
	{
		Delta = delta;

		return *this;
	}

	template<class T_Ret, class ... T_Args>
	T_Ret MI_MFP::Apply(void * pthis, T_Args ... args) const
	{
		typedef T_Ret(__MIC::*MFP)(T_Args...);

		MFP mfp;
		
		this->Convert(&mfp);

		return ((__MIC *)pthis->*mfp)(args...);
	}

	void * MI_MFP::Addressing(void * pthis) const
	{
		return (char *)pthis + Delta;//多继承成员函数指针 this 加上 delta 即可
	}

	void * MI_MFP::AutoAddressing(void * pthis) const
	{
		MI_MFP temp = *this;

		temp.SetCodePtr(&MI_MFP::GetThis);

		//寻址拿到调整后的this指针
		void* AdjustedThis = temp.Apply<void *>(pthis);

		return AdjustedThis;
	}

	template<class T, class T_Ret, class ... T_Args>
	bool MI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...)) const
	{
		if (sizeof(*p_mfp) == sizeof(MI_MFP))
		{
			*(void **)p_mfp = this->CodePtr;
			*(int *)((char *)p_mfp + sizeof(void *)) = this->Delta;

			return true;
		}
		return false;
	}

	bool MI_MFP::Equal(const MI_MFP & that) const
	{
		if (nullptr == &that)
		{
			return false;
		}
		return this->SI_MFP::Equal(that) && that.Delta == this->Delta;
	}

#pragma endregion

#pragma region 虚继承成员函数指针

	VI_MFP::VI_MFP() :MI_MFP(), VBTableIndex(0)
	{

	}

	template<class T, class T_Ret, class ...T_Args>
	VI_MFP::VI_MFP(T_Ret(T::*mfp)(T_Args...))
	{
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	VI_MFP & VI_MFP::operator=(T_Ret(T::*mfp)(T_Args...))
	{
		return this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	VI_MFP & VI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_VI_MFP_CHECK__(mfp);

		void * p_mfp = &mfp;
		this->CodePtr = *(void **)p_mfp;
		this->Delta = *(int *)((char *)p_mfp + sizeof(void *));
		this->VBTableIndex = *(int *)((char *)p_mfp + sizeof(void *)+sizeof(int));
		return *this;
	}

	int VI_MFP::GetVBTableIndex() const
	{
		return VBTableIndex;
	}

	VI_MFP & VI_MFP::SetVBTableIndex(int vbtable_index)
	{
		VBTableIndex = vbtable_index;
		return *this;
	}

	template<class T_Ret, class T, class ... T_Args>
	T_Ret VI_MFP::Apply(T * pthis, T_Args ... args) const
	{
		//虚继承函数指针在调用时，编译器需要知道 this 的类型，以便他去找vbptr
		//如果像 单继承成员函数指针和多继承成员函数指针那样通过中间类去调用的话，会导致找到错误的 vbptr
		//最后导致调用异常

		typedef T_Ret(T::*MFP)(T_Args...);

		MFP mfp;

		this->Convert(&mfp);

		return (pthis->*mfp)(args...);
	}

	template<class T>
	void * VI_MFP::Addressing(T * pthis) const
	{
		//虚基类表中，记录了 vbptr 到虚基类子对象的偏移量，也就是子类到虚基类子对象的偏移，数据类型是int，
		//也就说每个数据占四个字节，需要注意的是 VBTableIndex的单位是1字节，一般VBTableIndex的值是四的倍数
		//例如，子类的第一个虚基类偏移记录在 vbptr指定地址+4的地方(vbptr+0 至 vbptr+3 为 0), VBTableIndex就等于4
		//子类的第二个虚基类偏移记录在 vbptr指定地址+8的地方, VBTableIndex就等于8
		//作者其实不是很明白为什么这么设计，感觉很怪，

		//虚继承中，继承情况很复杂，vbptr 的位置只有编译器知道(编译期)，
		//我们是无法确定的(也许可以确定，但是作者截至目前为止还不知道如何确定 vbptr 的位置)

		//每个类的 vbptr位置是不同的，编译器根据 this 的类型来确定 vbptr 的位置
		//拿不到vbptr，就没法 根据 VBTableIndex 计算 虚基类子对象的偏移量，就拿不到调整后的this
		//所以，我们只能通过编译器来寻址 。具体参考 AutoAddressing 函数

		//假定类的第一项就是 vbptr，以下代码展示了如何寻址
		//int virtual_delta = 0;
		//if (this->VBTableIndex) 
		//{
		//	const char * vbptr = *(char**)pthis;

		//	virtual_delta = *(int*)(vbptr + this->VBTableIndex);
		//}

		//void * AdjustedThis = (char *)pthis + Delta + virtual_delta;

		//return AdjustedThis;

		//=================================================================================
		//目前无法手动计算寻址，为了防止错误的调用，我们直接调用AutoAddressing；
		return AutoAddressing(pthis);
	}

	template<class T>
	void * VI_MFP::AutoAddressing(T * pthis) const
	{
		VI_MFP temp = *this;

		temp.SetCodePtr(&VI_MFP::GetThis);
		//寻址拿到调整后的this指针
		T* AdjustedThis = temp.Apply<T *>(pthis);

		return AdjustedThis;
	}

	template<class T, class T_Ret, class ... T_Args>
	bool VI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...)) const
	{
		if (sizeof(*p_mfp) == sizeof(VI_MFP))
		{
			*(void **)p_mfp = this->CodePtr;
			*(int *)((char *)p_mfp + sizeof(void *)) = this->Delta;
			*(int *)((char *)p_mfp + sizeof(void *) + sizeof(int)) = this->VBTableIndex;

			return true;
		}
		return false;
	}

	bool VI_MFP::Equal(const VI_MFP & that) const
	{
		if (nullptr == &that)
		{
			return false;
		}
		return this->MI_MFP::Equal(that) && that.VBTableIndex == this->VBTableIndex;
	}

#pragma endregion

#pragma region 完整的成员函数指针

	Full_MFP::Full_MFP() :MI_MFP(), FVtorDisp(0), VBTableIndex(0)
	{
	}

	template<class T, class T_Ret, class ...T_Args>
	Full_MFP::Full_MFP(T_Ret(T::*mfp)(T_Args...))
	{
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	Full_MFP & Full_MFP::operator=(T_Ret(T::* mfp)(T_Args...))
	{
		return this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	Full_MFP & Full_MFP::Deassign(T_Ret(T::* mfp)(T_Args...))
	{
		__STATIC_ASSERT_FULL_MFP_CHECK__(mfp);

		this->CodePtr = *(void **)p_mfp;
		this->Delta = *(int *)((char *)p_mfp + sizeof(void *));
		this->FVtorDisp = *(int *)((char *)p_mfp + sizeof(void *) + sizeof(int));
		this->VBTableIndex = *(int *)((char *)p_mfp + sizeof(void *) + sizeof(int)*2);
		return *this;
	}

	template<class T_Ret, class ...T_Args>
	T_Ret Full_MFP::Apply(void * pthis, T_Args ...args) const
	{
		//与虚继承成员函数指针不同,完整的成员函数指针可以像单继承和多继承成员函数指针一样
		//通过中间类this来调用,因为 由于FVtorDisp的存在,编译器可以正确的找到vbptr。

		//唯一需要注意的是，返回值与参数列表的类型要与实际的函数签名一致，如果不一致，可能会导致调用异常

		typedef T_Ret(__UND::*MFP)(T_Args...);

		MFP mfp;

		this->Convert(&mfp);

		return ((__UND *)pthis->*mfp)(args...);
	}

	void * Full_MFP::Addressing(void * pthis) const
	{
		//因为完整成员函数指针可以通过 FVtorDisp 找到 vbptr,所以并不需要 this 的类型,区别于 虚继承成员函数指针的寻址方式

		//virtual_delta表示子类到虚基类子对象的偏移量
		int virtual_delta = 0;

		if (this->VBTableIndex)
		{
			//先通过 FVtorDisp 找到 vbptr
			const char * vbptr = *(char **)((char *)pthis + this->Delta + this->FVtorDisp);
			//然后从虚基类表中取到虚基类偏移量
			virtual_delta = *(int*)(vbptr + this->VBTableIndex);
		}

		// 最后计算调整后的this
		// this 到基类子对象 this的偏移量 = 
		// this调整值(Delta) + vbtpr到基类子对象的偏移量(virtual_delta)
		// 整个计算过程为:1.通过this 找到虚基类表 2.通过虚基类表计算得到虚基类表到基类子对象的偏移 3.最后通过偏移找到基类子对象

		void * AdjustedThis = (char *)pthis + Delta /*+ FVtorDisp */ + virtual_delta;//不应该加上FVtorDisp,之前的理解是错误的

		return AdjustedThis;

	}

	void * Full_MFP::AutoAddressing(void * pthis) const
	{
		Full_MFP temp = *this;

		temp.SetCodePtr(&Full_MFP::GetThis);

		//寻址拿到调整后的this指针
		void* AdjustedThis = temp.Apply<void *>(pthis);

		return AdjustedThis;
	}

	int Full_MFP::GetFVtorDisp() const
	{
		return FVtorDisp;
	}

	Full_MFP & Full_MFP::SetFVtorDisp(int fVtorDisp)
	{
		FVtorDisp = fVtorDisp;
		return *this;
	}

	int Full_MFP::GetVBTableIndex() const
	{
		return VBTableIndex;
	}

	Full_MFP & Full_MFP::SetVBTableIndex(int vbtable_index)
	{
		VBTableIndex = vbtable_index;
		return *this;
	}

	template<class T, class T_Ret, class ... T_Args>
	bool Full_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...)) const
	{
		if (sizeof(*p_mfp) == sizeof(Full_MFP))
		{
			*(void **)p_mfp = this->CodePtr;
			*(int *)((char *)p_mfp + sizeof(void *)) = this->Delta;
			*(int *)((char *)p_mfp + sizeof(void *) + sizeof(int)) = this->FVtorDisp;
			*(int *)((char *)p_mfp + sizeof(void *) + sizeof(int) * 2) = this->VBTableIndex;

			return true;
		}
		return false;
	}

	bool Full_MFP::Equal(const Full_MFP & that) const
	{
		if (nullptr == &that)
		{
			return false;
		}
		return this->MI_MFP::Equal(that) && that.FVtorDisp == this->FVtorDisp && that.VBTableIndex == this->VBTableIndex;
	}

#pragma endregion

#pragma endregion

	//兼容所有类型成员函数指针
	class MemberFunctionPointer :public Full_MFP
	{
	protected:
		MFP_TYPE Type;//标记自身的实际类型

		//虚继承成员函数指针第三个成员是VBTableIndex，而完整的成员函数指针第四个成员才是VBTableIndex
		//所以需要做一下特殊处理
		VI_MFP To_VI_MFP() const
		{
			VI_MFP vi_mfp;
			vi_mfp.SetCodePtr(this->CodePtr);
			vi_mfp.SetDelta(this->Delta);
			vi_mfp.SetVBTableIndex(this->VBTableIndex);

			return vi_mfp;
		}

	public:
		MemberFunctionPointer() :Full_MFP(), Type(MFP_TYPE::Invalid)
		{

		}

		template<class T, class T_Ret, class ...T_Args>
		MemberFunctionPointer(T_Ret(T::*mfp)(T_Args...))
		{
			this->Deassign(mfp);
		}

		template<class T, class T_Ret, class ...T_Args>
		MemberFunctionPointer & operator=(T_Ret(T::*mfp)(T_Args...))
		{
			return this->Deassign(mfp);
		}

		template<class T, class T_Ret, class ...T_Args>
		MemberFunctionPointer & Deassign(T_Ret(T::*mfp)(T_Args...))
		{
			__STATIC_ASSERT_MFP_CHECK__(mfp);

			union U
			{
				U() :CodePtr(nullptr), i1(0), i2(0), i3(0) {}
				struct
				{
					void * CodePtr;
					int i1;
					int i2;
					int i3;
				};
				T_Ret(T::*_mfp)(T_Args...);
			} u;

			u._mfp = mfp;

			MFP_TYPE type = (MFP_TYPE)sizeof(mfp);
			
			switch (type)
			{
			case MFP_TYPE::SI_MFP:
			case MFP_TYPE::MI_MFP:
			case MFP_TYPE::Full_MFP:
			{
				this->CodePtr = u.CodePtr;
				this->Delta = u.i1;
				this->FVtorDisp = u.i2;
				this->VBTableIndex = u.i3;
				this->Type = type;
				return *this;
			}
			case MFP_TYPE::VI_MFP:
			{
				this->CodePtr = u.CodePtr;
				this->Delta = u.i1;
				this->FVtorDisp = u.i3;
				this->VBTableIndex = u.i2;
				this->Type = type;
				return *this;
			}
			case MFP_TYPE::Invalid:
			default:
				return *(MemberFunctionPointer *)nullptr;
			}
		}

		MemberFunctionPointer & Reset()
		{
			this->CodePtr = nullptr;
			this->Delta = 0;
			this->FVtorDisp = 0;
			this->VBTableIndex = 0;
			this->Type = MFP_TYPE::Invalid;
			return *this;
		}

		MFP_TYPE GetType() const
		{
			return Type;
		}

		template<class T_Ret, class T, class ... T_Args>
		T_Ret Apply(T * pthis, T_Args ... args) const
		{
			switch (Type)
			{
			case Private::MFP_TYPE::SI_MFP:
			{
				return this->SI_MFP::Apply<T_Ret>(pthis,args...);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->MI_MFP::Apply<T_Ret>(pthis, args...);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Apply<T_Ret>(pthis, args...);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->Full_MFP::Apply<T_Ret>(pthis, args...);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//可以考虑抛异常
				return T_Ret();//不应该到这里
			}
		}

		template<class T>
		void * Addressing(T * pthis) const
		{
			switch (Type)
			{
			case Private::MFP_TYPE::SI_MFP:
			{
				return this->SI_MFP::Addressing(pthis);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->MI_MFP::Addressing(pthis);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Addressing(pthis);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->Full_MFP::Addressing(pthis);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//可以考虑抛异常
				return nullptr;//不应该到这里
			}
		}

		template<class T>
		void * AutoAddressing(T * pthis) const
		{
			switch (Type)
			{
			case Private::MFP_TYPE::SI_MFP:
			{
				return this->SI_MFP::AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->MI_MFP::AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->Full_MFP::AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//可以考虑抛异常
				return nullptr;//不应该到这里
			}
		}

		//转换为C++原生的成员函数指针，请注意返回值与参数列表
		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const
		{
			MFP_TYPE DstType = (MFP_TYPE)sizeof(*p_mfp);
			if (DstType != Type)
			{
				//目标类型与实际类型不服，强行转换的话可能会存在数据丢失造成调用异常
			}
			switch (Type)
			{
			case Private::MFP_TYPE::SI_MFP:
			{
				return this->SI_MFP::Convert(p_mfp);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->MI_MFP::Convert(p_mfp);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Convert(p_mfp);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->Full_MFP::Convert(p_mfp);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//可以考虑抛异常
				return false;//不应该到这里
			}
		}

		bool Equal(const MemberFunctionPointer & that) const
		{
			if (that.Type != this->Type)
			{
				return false;
			}
			return this->Full_MFP::Equal(that);
		}

		bool operator==(const MemberFunctionPointer & that) const
		{
			return this->Equal(that);
		}
	};

}



