#pragma once

//�����õ��˲�����ģ���������������Ҫ֧�� C++11 ��׼
#if __cplusplus < 199711L
#error [Private.MemberFunctionPointer]��Ҫ������֧�� C++11 ��׼
#endif

/*
* ��vc������,Ӱ���Ա����ָ��ı�ʾ��ʽ�ķ�ʽ������
* 1.���������� /vmg /vmb
* https://msdn.microsoft.com/zh-cn/library/yad46a6z.aspx
* 2.#pragma pointers_to_members( pointer-declaration, [most-general-representation] ) [C++ ר��]
* https://msdn.microsoft.com/zh-cn/library/83cch5a6.aspx
* 3.�̳йؼ��� __single_inheritance,__multiple_inheritance,__virtual_inheritance [Microsoft ר��]
* https://msdn.microsoft.com/zh-cn/library/ck561bfk.aspx
* ���ַ�ʽ�����ȼ�����
*/

/*
* ����,��vc������,��Ա����ָ��ı�ʾ��ʽһ��������
* 1.���̳���ĳ�Ա����ָ��,���� 1 ������:������ʵ�ʵ�ַ(�����麯����˵���� vcall thunk(virtual call thunk,����������� �������ת�� ). ����ο� ������ vcall thunk��
* 2.��̳���ĳ�Ա����ָ��,���� 2 ������: ������ʵ�ʵ�ַ,this ����ֵ(�й�this����ֵ������Բο���̳е��ڴ沼��)
* 3.��̳���ĳ�Ա����ָ��,���� 3 ������: ������ʵ�ʵ�ַ,this ����ֵ,����������(�����麯���� vbptr != vfptr),����ο�����Ĵ���ʵ�ֺ�ע��
* 4.�����ĳ�Ա����ָ��,���� 3 ������: ������ʵ�ʵ�ַ,this ����ֵ,������ƫ����(�� this �� ������),����������
* ע�� ��̳���ĳ�Ա����ָ�� �� �����ĳ�Ա����ָ�� ֮����ڵĲ���, ��̳���ĳ�Ա����ָ��� 3 �������� [����������],�������ĳ�Ա����ָ��� 4 �����ݲ���
* �����ָ��ṹ�����ο� ��Ա����ָ��ṹ����
*/

/* ���� vcall thunk
* ʵ����vcall thunk ����ʵ�ʺ�������ת����,�����ڲ�����ת���麯�����е�slotλ�ö�Ӧ�ĺ�����ַ.
* ΢��ʹ�� vcall thunk ������֧���麯��,���е�ͬ�����麯��(������麯����������д����麯��)����Ӧͬһ��vcall,
* ֮����ͬ�����麯�����Ժ�vcall thunk ���һ,����Ϊ,�����Ǹ��໹������,ͬ�����麯�����麯�����е�λ����һ�µ�)
* �ٸ�������˵����,�����е��麯�� A �ڸ�����麯�������ǵ� 2 slot,��ô������д A ������,�麯�� A ��������麯������
* ��Ҳ�� 2 slot,��Ϊ ������麯�����Ը���̳ж���,����ֻ����չ�麯����(����������Լ����麯��),
* ����ı�̳й������麯����,�е�������ļ̳�.
* ÿ�� vcall thunk ��Ӧ�� slot �ǹ̶���,Ȼ������ this (���ߵ������this)�ҵ��麯����,
* ���㷽ʽΪ:const char * vbptr = *(char**)pthis , ����ԭ����� ��ĵ�һ����Ա���� vfptr(��������麯��)
* ������麯�����е�slotλ�û�ȡ��ʵ�ʵĺ�����ַ������ת
*/

#pragma region ��̬���Ժ�
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
	//�޼̳���
	class __NIC {};
	//���̳���
	class __single_inheritance __SIC : __Base{};
	//��̳���
	class __multiple_inheritance __MIC : __Base, __NIC{};
	//��̳���
	class __virtual_inheritance __VIC : virtual __Base{};

	//δ�������,�ڶ�����֮ǰ������ָ�����Ա��ָ��,�����ĳ�Աָ����һ��������ָ�롣
	//��Ϊ�ڶ�����֮ǰ����������֪����ļ̳з�ʽ����֪�������Աָ�����ѱ�ʾ��ʽ��
	//���Ծ��������ĳ�Ա����ָ������ʾ���Լ���δ֪���������
	//���Բο� https://msdn.microsoft.com/zh-cn/library/yad46a6z.aspx
	class __UND;

#pragma endregion

#pragma region  ��Ա����ָ��ṹ����
	//���̳г�Ա����ָ��
	class SI_MFP
	{
	protected:
		//ֻ����ʵ�ʺ�����ַ
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

		//���ó�Ա����ָ�룬��Ҫ��֤����ֵ������б���ʵ�ʺ���ǩ��һ�£�������ܻᵼ�µ����쳣
		template<class T_Ret, class ... T_Args>
		T_Ret Apply(void * pthis, T_Args ... args) const;

		//��ʵ�ϣ����ǲ���Ҫ���ı�������������Ѱַ��
		//����ֻ��Ҫ֪����ͨ����Ա����ָ����к������õ�ʱ��
		//���������Զ����Ѱַ
		//Ҳ����˵���ڵ���ʵ�ʵĳ�Ա����ʱ��this ָ�����Ѿ����й�������
		//���ԣ����ǵ�Ѱַ����ԭ���ǣ��޸� ԭʼ�ĳ�Ա����ָ�� �� [ʵ�ʺ�����ַ]
		//���������ַָ�� �����Զ����һ���������������ֻ��һ���򵥵� return this;
		//����ֻ�޸��� [ʵ�ʺ�����ַ]����Ա����ָ�����������û���޸�
		//���Ա�������Ѱַ������޸�ǰ��Ѱַ�����һ����
		//[ʵ�ʺ�����ַ] ָ��ĺ������Խ� ������Ѱַ��Ľ�� this ���ء�
		//���ǲ���Ҫ���ĳ�Ա����ָ�����ɣ�Ҳ����Ҫ�������Ѱַ��ֻ��Ҫ��֤this����ȷ����
		//��һ�б���������������
		void * Addressing(void * pthis) const;

		void * AutoAddressing(void * pthis) const;

		//ת��ΪC++ԭ���ĳ�Ա����ָ�룬��ע�ⷵ��ֵ������б�
		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const SI_MFP & that) const;
	};

	//��̳г�Ա����ָ��
	class MI_MFP :public SI_MFP
	{
	protected:
		//�ڵ��̳г�Ա����ָ������϶��� this ����ֵ
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

	//��̳г�Ա����ָ��
	class VI_MFP :public MI_MFP
	{
	protected:
		//�ڶ�̳г�Ա����ָ��̳��϶�������������
		//��ֵָʾ��Ӧ����[������]���ĸ�λ��(���ֽ�Ϊ��λ)��ʼȡֵ(ȡint)�õ�[�����]��ƫ����
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

		//Ѱַ���������ǻ����Ӷ������Բ� ���� T*
		template<class T>
		void * Addressing(T * pthis) const;

		//Ѱַ���������ǻ����Ӷ������Բ� ���� T*
		template<class T>
		void * AutoAddressing(T * pthis) const;

		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const;

		bool Equal(const VI_MFP & that) const;

	};

	//�����ĳ�Ա����ָ��
	//֮���Բ�ֱ�Ӽ̳� VI_MFP ,����Ϊ �����ĳ�Ա����ָ�� ���ĸ���Ա���� VBTableIndex
	class Full_MFP :public MI_MFP
	{
	protected:
		//this �� vbptr ��ƫ����,������ this(����this) �������Ӷ����ƫ����
		int FVtorDisp;
		//����������
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

	//���¶��Կ���ȷ��ʵ�ʵĳ�Ա����ָ���С������������һ��
	//�����һ�£���˵������VS����������΢��������Գ�Ա����ָ���ʵ�ַ�ʽ�����˸ı�(�⼸��������)
	//δ����ʱ,����ľ�̬���Կ��ܻ���ʾʧ��,���õ���,�������ǿ��֢,����ע�͵�.�Ҿ���ǿ��֢.
	__STATIC_ASSERT_MFP_SIZE_CHECK__;

	//��Ա����ָ�����͵�ö��ֵ
	enum class MFP_TYPE :unsigned char
	{
		//��Ч��Ա����ָ��
		Invalid = 0,
		//���̳���ĳ�Ա����ָ��
		SI_MFP = sizeof(void(__SIC::*)()),
		//��̳���ĳ�Ա����ָ��
		MI_MFP = sizeof(void(__MIC::*)()),
		//��̳���ĳ�Ա����ָ��
		VI_MFP = sizeof(void(__VIC::*)()),
		//������Ա����ָ��
		Full_MFP = sizeof(void(__UND::*)()),
	};

#pragma region ��Ա����ָ��ṹ ʵ��

#pragma region ���̳г�Ա����ָ��

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
		//���ܴ���ĳ�Ա����ָ�������������ͣ�����ֻҪCodePtr,Ҳ���ǳ�Ա����ָ��ĵ�һ����
		//��Ա����ָ��ĵ�һ�������ݣ�Ҳ���Ǻ���ʵ�ʵ�ַ
		void * p_mfp = &mfp;
		this->CodePtr = *(void **)p_mfp;
		return *this;
	}

	template<class T_Ret, class ... T_Args>
	T_Ret SI_MFP::Apply(void * pthis, T_Args ... args) const
	{
		//���ó�Ա����ָ�룬��Ҫ���ö��󣬼� this ָ��
		//���ǲ���Ҫ���� this �ľ������ͣ�ֻ��Ҫ������������ĳ�Ա����ָ��
		//ת���м��� __SIC �ĳ�Ա����ָ�룬Ȼ�󽫴����pthisת����ǰ��ָ�룬���ͨ����ǰ��ָ����ü���
		//��Ϊ __SIC ����ʱָ���� __single_inheritance,������ __SIC �Ĵ�С��������ĳ�Ա����ָ���С��ͬ
		//ת��ʱ���ᶪʧ����

		typedef T_Ret(__SIC::*MFP)(T_Args...);

		MFP mfp;
		
		this->Convert(&mfp);

		return ((__SIC *)pthis->*mfp)(args...);
	}

	//��ʵ�ϣ����ǲ���Ҫ���ı�������������Ѱַ��
	//����ֻ��Ҫ֪����ͨ����Ա����ָ����к������õ�ʱ��
	//���������Զ����Ѱַ
	//Ҳ����˵���ڵ���ʵ�ʵĳ�Ա����ʱ��this ָ�����Ѿ����й�������
	//���ԣ����ǵ�Ѱַ����ԭ���ǣ��޸� ԭʼ�ĳ�Ա����ָ�� �� [ʵ�ʺ�����ַ]
	//���������ַָ�� �����Զ����һ���������������ֻ��һ���򵥵� return this;
	//����ֻ�޸��� [ʵ�ʺ�����ַ]����Ա����ָ�����������û���޸�
	//���Ա�������Ѱַ������޸�ǰ��Ѱַ�����һ����
	//[ʵ�ʺ�����ַ] ָ��ĺ������Խ� ������Ѱַ��Ľ�� this ���ء�
	//���ǲ���Ҫ���ĳ�Ա����ָ�����ɣ�Ҳ����Ҫ�������Ѱַ��ֻ��Ҫ��֤this����ȷ����
	//��һ�б���������������
	void * SI_MFP::Addressing(void * pthis) const
	{
		return pthis;//���̳к���ָ��this����Ҫ������
	}

	void * SI_MFP::AutoAddressing(void * pthis) const
	{
		SI_MFP temp = *this;
		temp.SetCodePtr(&SI_MFP::GetThis);

		//Ѱַ�õ��������thisָ��
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

#pragma region ��̳г�Ա����ָ��

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
		return (char *)pthis + Delta;//��̳г�Ա����ָ�� this ���� delta ����
	}

	void * MI_MFP::AutoAddressing(void * pthis) const
	{
		MI_MFP temp = *this;

		temp.SetCodePtr(&MI_MFP::GetThis);

		//Ѱַ�õ��������thisָ��
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

#pragma region ��̳г�Ա����ָ��

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
		//��̳к���ָ���ڵ���ʱ����������Ҫ֪�� this �����ͣ��Ա���ȥ��vbptr
		//����� ���̳г�Ա����ָ��Ͷ�̳г�Ա����ָ������ͨ���м���ȥ���õĻ����ᵼ���ҵ������ vbptr
		//����µ����쳣

		typedef T_Ret(T::*MFP)(T_Args...);

		MFP mfp;

		this->Convert(&mfp);

		return (pthis->*mfp)(args...);
	}

	template<class T>
	void * VI_MFP::Addressing(T * pthis) const
	{
		//�������У���¼�� vbptr ��������Ӷ����ƫ������Ҳ�������ൽ������Ӷ����ƫ�ƣ�����������int��
		//Ҳ��˵ÿ������ռ�ĸ��ֽڣ���Ҫע����� VBTableIndex�ĵ�λ��1�ֽڣ�һ��VBTableIndex��ֵ���ĵı���
		//���磬����ĵ�һ�������ƫ�Ƽ�¼�� vbptrָ����ַ+4�ĵط�(vbptr+0 �� vbptr+3 Ϊ 0), VBTableIndex�͵���4
		//����ĵڶ��������ƫ�Ƽ�¼�� vbptrָ����ַ+8�ĵط�, VBTableIndex�͵���8
		//������ʵ���Ǻ�����Ϊʲô��ô��ƣ��о��ܹ֣�

		//��̳��У��̳�����ܸ��ӣ�vbptr ��λ��ֻ�б�����֪��(������)��
		//�������޷�ȷ����(Ҳ�����ȷ�����������߽���ĿǰΪֹ����֪�����ȷ�� vbptr ��λ��)

		//ÿ����� vbptrλ���ǲ�ͬ�ģ����������� this ��������ȷ�� vbptr ��λ��
		//�ò���vbptr����û�� ���� VBTableIndex ���� ������Ӷ����ƫ���������ò����������this
		//���ԣ�����ֻ��ͨ����������Ѱַ ������ο� AutoAddressing ����

		//�ٶ���ĵ�һ����� vbptr�����´���չʾ�����Ѱַ
		//int virtual_delta = 0;
		//if (this->VBTableIndex) 
		//{
		//	const char * vbptr = *(char**)pthis;

		//	virtual_delta = *(int*)(vbptr + this->VBTableIndex);
		//}

		//void * AdjustedThis = (char *)pthis + Delta + virtual_delta;

		//return AdjustedThis;

		//=================================================================================
		//Ŀǰ�޷��ֶ�����Ѱַ��Ϊ�˷�ֹ����ĵ��ã�����ֱ�ӵ���AutoAddressing��
		return AutoAddressing(pthis);
	}

	template<class T>
	void * VI_MFP::AutoAddressing(T * pthis) const
	{
		VI_MFP temp = *this;

		temp.SetCodePtr(&VI_MFP::GetThis);
		//Ѱַ�õ��������thisָ��
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

#pragma region �����ĳ�Ա����ָ��

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
		//����̳г�Ա����ָ�벻ͬ,�����ĳ�Ա����ָ������񵥼̳кͶ�̳г�Ա����ָ��һ��
		//ͨ���м���this������,��Ϊ ����FVtorDisp�Ĵ���,������������ȷ���ҵ�vbptr��

		//Ψһ��Ҫע����ǣ�����ֵ������б������Ҫ��ʵ�ʵĺ���ǩ��һ�£������һ�£����ܻᵼ�µ����쳣

		typedef T_Ret(__UND::*MFP)(T_Args...);

		MFP mfp;

		this->Convert(&mfp);

		return ((__UND *)pthis->*mfp)(args...);
	}

	void * Full_MFP::Addressing(void * pthis) const
	{
		//��Ϊ������Ա����ָ�����ͨ�� FVtorDisp �ҵ� vbptr,���Բ�����Ҫ this ������,������ ��̳г�Ա����ָ���Ѱַ��ʽ

		//virtual_delta��ʾ���ൽ������Ӷ����ƫ����
		int virtual_delta = 0;

		if (this->VBTableIndex)
		{
			//��ͨ�� FVtorDisp �ҵ� vbptr
			const char * vbptr = *(char **)((char *)pthis + this->Delta + this->FVtorDisp);
			//Ȼ�����������ȡ�������ƫ����
			virtual_delta = *(int*)(vbptr + this->VBTableIndex);
		}

		// ������������this
		// this �������Ӷ��� this��ƫ���� = 
		// this����ֵ(Delta) + vbtpr�������Ӷ����ƫ����(virtual_delta)
		// �����������Ϊ:1.ͨ��this �ҵ������� 2.ͨ�����������õ������������Ӷ����ƫ�� 3.���ͨ��ƫ���ҵ������Ӷ���

		void * AdjustedThis = (char *)pthis + Delta /*+ FVtorDisp */ + virtual_delta;//��Ӧ�ü���FVtorDisp,֮ǰ������Ǵ����

		return AdjustedThis;

	}

	void * Full_MFP::AutoAddressing(void * pthis) const
	{
		Full_MFP temp = *this;

		temp.SetCodePtr(&Full_MFP::GetThis);

		//Ѱַ�õ��������thisָ��
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

	//�����������ͳ�Ա����ָ��
	class MemberFunctionPointer :public Full_MFP
	{
	protected:
		MFP_TYPE Type;//��������ʵ������

		//��̳г�Ա����ָ���������Ա��VBTableIndex���������ĳ�Ա����ָ����ĸ���Ա����VBTableIndex
		//������Ҫ��һ�����⴦��
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
				//���Կ������쳣
				return T_Ret();//��Ӧ�õ�����
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
				//���Կ������쳣
				return nullptr;//��Ӧ�õ�����
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
				//���Կ������쳣
				return nullptr;//��Ӧ�õ�����
			}
		}

		//ת��ΪC++ԭ���ĳ�Ա����ָ�룬��ע�ⷵ��ֵ������б�
		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...)) const
		{
			MFP_TYPE DstType = (MFP_TYPE)sizeof(*p_mfp);
			if (DstType != Type)
			{
				//Ŀ��������ʵ�����Ͳ�����ǿ��ת���Ļ����ܻ�������ݶ�ʧ��ɵ����쳣
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
				//���Կ������쳣
				return false;//��Ӧ�õ�����
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



