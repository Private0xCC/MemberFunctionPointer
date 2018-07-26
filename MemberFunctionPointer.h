#pragma once

//�����õ��˲�����ģ���������������Ҫ֧�� C++11 ��׼
#if __cplusplus < 199711L
#error ��Ҫ������֧�� C++11 ��׼
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
*
* ����,��vc������,��Ա����ָ��ı�ʾ��ʽһ��������
*/

#pragma region ��̬���Ժ�
#define __STATIC_ASSERT_MFP_SIZE_CHECK__ \
static_assert(sizeof(void(__SIC::*)()) == sizeof(SI_MFP), "An unexpected mistake,[single inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__MIC::*)()) == sizeof(MI_MFP), "An unexpected mistake,[multiple inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__VIC::*)()) == sizeof(VI_MFP), "An unexpected mistake,[virtual inheritance member function pointer]  size is not consistent with expectations.");\
static_assert(sizeof(void(__UND::*)()) == sizeof(Full_MFP), "An unexpected mistake,[Full member function pointer]  size is not consistent with expectations.");

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
	//ʵ�ַ�ʽ��C++��׼��һ�£��Լ�ʵ�ָú�����Ϊ�˼����ⲿ����
	void * __memcpy(void *dst, const void *src, unsigned int len)
	{
		if (nullptr == dst || nullptr == src)
		{
			return nullptr;
		}

		void *ret = dst;

		if (dst <= src || (char *)dst >= (char *)src + len)
		{
			//û���ڴ��ص����ӵ͵�ַ��ʼ����
			while (len--)
			{
				*(char *)dst = *(char *)src;
				dst = (char *)dst + 1;
				src = (char *)src + 1;
			}
		}
		else
		{
			//���ڴ��ص����Ӹߵ�ַ��ʼ����
			src = (char *)src + len - 1;
			dst = (char *)dst + len - 1;
			while (len--)
			{
				*(char *)dst = *(char *)src;
				dst = (char *)dst - 1;
				src = (char *)src - 1;
			}
		}
		return ret;
	}

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

#pragma region  MFP ����
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
		bool Convert(T_Ret(T::** p_mfp)(T_Args...));
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
		bool Convert(T_Ret(T::** p_mfp)(T_Args...));
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

		template<class T>
		T * Addressing(T * pthis) const;

		template<class T>
		T * AutoAddressing(T * pthis) const;

		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...));

	};

	//�����ĳ�Ա����ָ��
	//֮���Բ�ֱ�Ӽ̳� VI_MFP ,����Ϊ �����ĳ�Ա����ָ�� ���ĸ���Ա���� VBTableIndex
	class Full_MFP :public MI_MFP
	{
	protected:
		//this �� vbptr ��ƫ����
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
		bool Convert(T_Ret(T::** p_mfp)(T_Args...));

#pragma region Getter/Setter

		int GetFVtorDisp() const;

		Full_MFP & SetFVtorDisp(int fVtorDisp);

		int GetVBTableIndex() const;

		Full_MFP & SetVBTableIndex(int vbtable_index);
#pragma endregion
	};

#pragma endregion

	//һ�¶��Կ���ȷ��ʵ�ʵĳ�Ա����ָ���С������������һ��,�����һ�£���˵������VS����������΢���������Գ�Ա����ָ���ʵ�ַ�ʽ�����˸ı�(�⼸��������)
	//δ����ʱ,����ľ�̬���Կ��ܻ���ʾʧ��,���õ���,�������ǿ��֢,����ע�͵�.�Ҿ���ǿ��֢.
	//__STATIC_ASSERT_MFP_SIZE_CHECK__;

	//��Ա����ָ�����͵�ö��ֵ
	enum class MFP_TYPE :unsigned char
	{
		Invalid = 0,
		SI_MFP = sizeof(void(__SIC::*)()),
		MI_MFP = sizeof(void(__MIC::*)()),
		VI_MFP = sizeof(void(__VIC::*)()),
		Full_MFP = sizeof(void(__UND::*)()),
	};

#pragma region MFP ʵ��

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
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	SI_MFP & SI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_SI_MFP_CHECK__(mfp);

		__memcpy(this, &mfp, sizeof(SI_MFP));

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
		__memcpy(&CodePtr, &mfp, sizeof(CodePtr));

		return *this;
	}

	template<class T_Ret, class ... T_Args>
	T_Ret SI_MFP::Apply(void * pthis, T_Args ... args) const
	{
		//���ó�Ա����ָ�룬��Ҫ���ö��󣬼� this ָ��
		//���ǲ���Ҫ���� this �ľ������ͣ�ֻ��Ҫ������������ĳ�Ա����ָ��
		//ת�ɵ�ǰ�� __SIC �ĳ�Ա����ָ�룬Ȼ�󽫴����pthisת����ǰ��ָ�룬���ͨ����ǰ��ָ����ü���
		//��Ϊ __SIC ����ʱָ���� __single_inheritance,������ __SIC �Ĵ�С��������ĳ�Ա����ָ���С��ͬ
		//ת��ʱ���ᶪʧ����

		typedef T_Ret(__SIC::*MFP)(T_Args...);

		MFP mfp;
		__memcpy(&mfp, this, sizeof(MFP));

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

	template<class T, class T_Ret, class ... T_Args>
	bool SI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...))
	{
		if (sizeof(*p_mfp) == sizeof(SI_MFP))
		{
			__memcpy(p_mfp, this, sizeof(SI_MFP));
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
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	MI_MFP & MI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_MI_MFP_CHECK__(mfp);

		__memcpy(this, &mfp, sizeof(*this));

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
		__memcpy(&mfp, this, sizeof(MFP));

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
	bool MI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...))
	{
		if (sizeof(*p_mfp) == sizeof(MI_MFP))
		{
			__memcpy(p_mfp, this, sizeof(MI_MFP));
			return true;
		}
		return false;
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
		this->Deassign(mfp);
	}

	template<class T, class T_Ret, class ...T_Args>
	VI_MFP & VI_MFP::Deassign(T_Ret(T::*mfp)(T_Args...))
	{
		__STATIC_ASSERT_VI_MFP_CHECK__(mfp);

		__memcpy(this, &mfp, sizeof(VI_MFP));

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
		__memcpy(&mfp, this, sizeof(MFP));

		return (pthis->*mfp)(args...);
	}

	template<class T>
	T * VI_MFP::Addressing(T * pthis) const
	{
		//�������У���¼�� vbptr ��������Ӷ����ƫ������Ҳ�������ൽ������Ӷ����ƫ�ƣ�����������int��
		//Ҳ��˵ÿ������ռ�ĸ��ֽڣ���Ҫע����� VBTableIndex�ĵ�λ��1�ֽڣ�һ��VBTableIndex��ֵ���ĵı���
		//���磬����ĵ�һ�������ƫ�Ƽ�¼�� vbptrָ����ַ+4�ĵط�(vbptr+0 �� vbptr+3 Ϊ 0), VBTableIndex�͵���4
		//����ĵڶ��������ƫ�Ƽ�¼�� vbptrָ����ַ+8�ĵط�, VBTableIndex�͵���8
		//������ʵ���Ǻ�����Ϊʲô��ô��ƣ��о��ܹ֣�

		//��̳��У��̳�����ܸ��ӣ�vbptr ��λ��ֻ�б�����֪����
		//�������޷�ȷ����(Ҳ������ȷ�����������߽���ĿǰΪֹ����֪�����ȷ�� vbptr ��λ��)

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
		//Ϊ�˷�ֹ����ĵ��ã�����ֱ�ӵ���AutoAddressing��
		return AutoAddressing(pthis);
	}

	template<class T>
	T * VI_MFP::AutoAddressing(T * pthis) const
	{
		VI_MFP temp = *this;

		temp.SetCodePtr(&VI_MFP::GetThis);
		//Ѱַ�õ��������thisָ��
		T* AdjustedThis = temp.Apply<T *>(pthis);

		return AdjustedThis;
	}

	template<class T, class T_Ret, class ... T_Args>
	bool VI_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...))
	{
		if (sizeof(*p_mfp) == sizeof(VI_MFP))
		{
			__memcpy(p_mfp, this, sizeof(VI_MFP));
			return true;
		}
		return false;
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

		__memcpy(this, &mfp, sizeof(Full_MFP));

		return *this;
	}

	template<class T_Ret, class ...T_Args>
	T_Ret Full_MFP::Apply(void * pthis, T_Args ...args) const
	{
		//����̳г�Ա����ָ�벻ͬ,�����ĳ�Ա����ָ������񵥼̳кͶ�̳г�Ա����ָ��һ��
		//ͨ���м���this������,��Ϊ ����FVtorDisp�Ĵ���,������������ȷ���ҵ�vbptr��

		//Ψһ��Ҫע����ǣ�����ֵ������б�������Ҫ��ʵ�ʵĺ���ǩ��һ�£������һ�£����ܻᵼ�µ����쳣

		typedef T_Ret(__UND::*MFP)(T_Args...);

		MFP mfp;

		__memcpy(&mfp, this, sizeof(MFP));

		return ((__UND *)pthis->*mfp)(args...);
	}

	void * Full_MFP::Addressing(void * pthis) const
	{
		//vbptr �� ������Ӷ����ƫ���������������ൽ������Ӷ����ƫ����
		int virtual_delta = 0;

		if (this->VBTableIndex)
		{
			//��ͨ�� FVtorDisp �ҵ� vbptr
			const char * vbptr = *(char **)((char *)pthis + this->FVtorDisp);
			//Ȼ�����������ȡ�������ƫ����
			virtual_delta = *(int*)(vbptr + this->VBTableIndex);
		}

		//������
		void * AdjustedThis = (char *)pthis + Delta + FVtorDisp + virtual_delta;

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
	bool Full_MFP::Convert(T_Ret(T::** p_mfp)(T_Args...))
	{
		if (sizeof(*p_mfp) == sizeof(Full_MFP))
		{
			__memcpy(p_mfp, this, sizeof(Full_MFP));
			return true;
		}
		return false;
	}

#pragma endregion

#pragma endregion

	//�����������ͳ�Ա����ָ��
	class MemberFunctionPointer :public Full_MFP
	{
	protected:
		MFP_TYPE Type;//���������ʵ������

		SI_MFP To_SI_MFP() const
		{
			SI_MFP si_mfp;
			si_mfp.SetCodePtr(this->CodePtr);

			return si_mfp;
		}

		MI_MFP To_MI_MFP() const
		{
			MI_MFP mi_mfp;
			mi_mfp.SetCodePtr(this->CodePtr);
			mi_mfp.SetDelta(this->Delta);

			return mi_mfp;
		}

		VI_MFP To_VI_MFP() const
		{
			VI_MFP vi_mfp;
			vi_mfp.SetCodePtr(this->CodePtr);
			vi_mfp.SetDelta(this->Delta);
			vi_mfp.SetVBTableIndex(this->VBTableIndex);

			return vi_mfp;
		}

		Full_MFP To_Full_MFP() const
		{
			Full_MFP full_mfp;
			full_mfp.SetCodePtr(this->CodePtr);
			full_mfp.SetDelta(this->Delta);
			full_mfp.SetFVtorDisp(this->FVtorDisp);
			full_mfp.SetVBTableIndex(this->VBTableIndex);

			return full_mfp;
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
			this->Type = type;
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
				return *this;
			}
			case MFP_TYPE::VI_MFP:
			{
				this->CodePtr = u.CodePtr;
				this->Delta = u.i1;
				this->FVtorDisp = u.i3;
				this->VBTableIndex = u.i2;
				return *this;
			}
			case MFP_TYPE::Invalid:
			default:
				return *(MemberFunctionPointer *)nullptr;
			}
		}

		MFP_TYPE GetType()
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
				return this->To_SI_MFP().Apply<T_Ret>(pthis,args...);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->To_MI_MFP().Apply<T_Ret>(pthis, args...);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Apply<T_Ret>(pthis, args...);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->To_Full_MFP().Apply<T_Ret>(pthis, args...);
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
				return this->To_SI_MFP().Addressing(pthis);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->To_MI_MFP().Addressing(pthis);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Addressing(pthis);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->To_Full_MFP().Addressing(pthis);
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
				return this->To_SI_MFP().AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->To_MI_MFP().AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->To_Full_MFP().AutoAddressing(pthis);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//���Կ������쳣
				return nullptr;//��Ӧ�õ�����
			}
		}

		//ת��ΪC++ԭ���ĳ�Ա����ָ�룬��ע�ⷵ��ֵ������б�
		template<class T, class T_Ret, class ... T_Args>
		bool Convert(T_Ret(T::** p_mfp)(T_Args...))
		{
			switch (Type)
			{
			case Private::MFP_TYPE::SI_MFP:
			{
				return this->To_SI_MFP().Convert(p_mfp);
			}
			case Private::MFP_TYPE::MI_MFP:
			{
				return this->To_MI_MFP().Convert(p_mfp);
			}
			case Private::MFP_TYPE::VI_MFP:
			{
				return this->To_VI_MFP().Convert(p_mfp);
			}
			case Private::MFP_TYPE::Full_MFP:
			{
				return this->To_Full_MFP().Convert(p_mfp);
			}
			case Private::MFP_TYPE::Invalid:
			default:
				//���Կ������쳣
				return false;//��Ӧ�õ�����
			}
		}
	};

}


