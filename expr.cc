#include <TooN/TooN.h>

using namespace std;
namespace TooN{

template<class B, class P> struct MakeSlice;

template<class Expr> struct SliceExpr
{
	template<int I, class P> struct VLayout: public MakeSlice<SliceExpr<Expr>, P>
	{
		typedef P Precision;
		const int start, length;
		const Vector<Dynamic, Precision, Expr>& vec;
		
		int size() const
		{
			return length;
		}

		Precision operator[](int i) const
		{
			return vec[start + i];
		}
		
		VLayout(int start_, int length_, const Vector<Dynamic, Precision, Expr>& vec_)
		:start(start_),length(length_),vec(vec_)
		{
		}
	};
};

template<class Base, class Precision = typename Base::Precision> struct MakeSlice
{
	Vector<Dynamic, Precision, SliceExpr<Base> > slice(int start, int length)
	{
		typedef Vector<Dynamic,  Precision, Base> Derived;
		const Derived& derived = static_cast<Derived&>(*this);

		return typename SliceExpr<Base>::template VLayout<Dynamic, Precision>(start, length, derived);
	}

	typedef void* PointerType;
	typedef void  try_destructive_resize;
};

template<class P1, class P2, class B2> struct ScalarMulExpr
{
	typedef typename Internal::MultiplyType<P1,P2>::type Precision;

	template<int I, class P> struct VLayout: public MakeSlice<ScalarMulExpr<P1, P2, B2> >
	{
		typedef typename Internal::MultiplyType<P1,P2>::type Precision;
		const P1& mul;
		const Vector<Dynamic, P2, B2>& vec;
		
		int size() const
		{
			return vec.size();
		}

		Precision operator[](int i) const
		{
			return vec[i]*mul;
		}
		
		VLayout(const P1& m, const Vector<Dynamic, P2, B2>& vec_)
		:mul(m),vec(vec_)
		{
		}
	};
};

template<class P1, class P2, class B2> struct ScalarDivExpr
{
	typedef typename Internal::MultiplyType<P1,P2>::type Precision;

	template<int I, class P> struct VLayout: public MakeSlice<ScalarDivExpr<P1, P2, B2> >
	{
		typedef typename Internal::MultiplyType<P1,P2>::type Precision;
		const P1& div;
		const Vector<Dynamic, P2, B2>& vec;
		
		int size() const
		{
			return vec.size();
		}

		Precision operator[](int i) const
		{
			return vec[i]/div;
		}
		
		VLayout(const P1& d, const Vector<Dynamic, P2, B2>& vec_)
		:div(d),vec(vec_)
		{
		}
	};
};

template<class P1, class B1> struct NegExpr
{
	typedef P1 Precision;

	template<int I, class P> struct VLayout: public MakeSlice<NegExpr<P1, B1> >
	{
		typedef P1 Precision;
		const Vector<Dynamic, P1, B1>& vec;
		
		int size() const
		{
			return vec.size();
		}

		Precision operator[](int i) const
		{
			return -vec[i];
		}
		
		VLayout( const Vector<Dynamic, P1, B1>& vec_)
		:vec(vec_)
		{
		}
	};
};


template<class P1, class P2, class B1, class B2> 
struct AddExpr
{
	typedef typename Internal::AddType<P1,P2>::type Precision;
	typedef Vector<Dynamic, P1, B1> LHS;
	typedef Vector<Dynamic, P2, B2> RHS;

	template<int I,class P> struct VLayout: public MakeSlice<AddExpr<P1,P2,B1,B2> >
	{
		typedef AddExpr<P1,P2,B1,B2>::Precision Precision;
		const LHS& lhs;
		const RHS& rhs;
		
		Precision operator[](int i) const
		{
			return lhs[i] + rhs[i];
		}

		int size() const
		{
			return lhs.size();
		}

		VLayout(const LHS& lhs_, const RHS& rhs_)
		:lhs(lhs_),rhs(rhs_)
		{
			SizeMismatch<Dynamic,Dynamic>:: test(lhs.size(),rhs.size());
		}

	};
};


template<class P1, class P2, class B1, class B2> 
struct SubExpr
{
	typedef typename Internal::SubtractType<P1,P2>::type Precision;
	typedef Vector<Dynamic, P1, B1> LHS;
	typedef Vector<Dynamic, P2, B2> RHS;
	template<int I,class P> struct VLayout: public MakeSlice<SubExpr<P1,P2,B1,B2> >
	{
		typedef SubExpr<P1,P2,B1,B2>::Precision Precision;
		const LHS& lhs;
		const RHS& rhs;
		
		Precision operator[](int i) const
		{
			return lhs[i] - rhs[i];
		}

		int size() const
		{
			return lhs.size();
		}

		VLayout(const LHS& lhs_, const RHS& rhs_)
		:lhs(lhs_),rhs(rhs_)
		{
			SizeMismatch<Dynamic,Dynamic>:: test(lhs.size(),rhs.size());
		}

	};
};



template<class P1, class P2, class B1, class B2>
Vector<Dynamic, typename SubExpr<P1,P2,B1,B2>::Precision, SubExpr<P1,P2,B1,B2> > operator-(const Vector<Dynamic,P1,B1>& lhs, const Vector<Dynamic, P2, B2>& rhs)
{
	typedef SubExpr<P1,P2,B1,B2> Expr;
	typedef typename Expr::Precision Precision;
	return typename Expr::template VLayout<Dynamic, Precision>(lhs, rhs);
}


template<class P1, class P2, class B1, class B2>
Vector<Dynamic, typename AddExpr<P1,P2,B1,B2>::Precision, AddExpr<P1,P2,B1,B2> > operator+(const Vector<Dynamic,P1,B1>& lhs, const Vector<Dynamic, P2, B2>& rhs)
{
	typedef AddExpr<P1,P2,B1,B2> Expr;
	typedef typename Expr::Precision Precision;
	typedef typename Expr::template VLayout<Dynamic, Precision> Base;
	return Base(lhs, rhs);
}

template<class P1, class P2, class B2>
Vector<Dynamic, typename ScalarMulExpr<P1,P2,B2>::Precision, ScalarMulExpr<P1,P2,B2> > operator*(const P1& lhs, const Vector<Dynamic, P2, B2>& rhs)
{
	typedef ScalarMulExpr<P1,P2,B2> Expr;
	typedef typename Expr::Precision Precision;
	typedef typename Expr::template VLayout<Dynamic, Precision> Base;
	return Base(lhs, rhs);
}

template<class P1, class P2, class B2>
Vector<Dynamic, typename ScalarMulExpr<P1,P2,B2>::Precision, ScalarMulExpr<P1,P2,B2> > operator*(const Vector<Dynamic, P2, B2>& lhs, const P1& rhs)
{
	typedef ScalarMulExpr<P1,P2,B2> Expr;
	typedef typename Expr::Precision Precision;
	typedef typename Expr::template VLayout<Dynamic, Precision> Base;
	return Base(rhs, lhs);
}

template<class P1, class P2, class B2>
Vector<Dynamic, typename ScalarDivExpr<P1,P2,B2>::Precision, ScalarDivExpr<P1,P2,B2> > operator/(const Vector<Dynamic, P2, B2>& lhs, const P1& rhs)
{
	typedef ScalarDivExpr<P1,P2,B2> Expr;
	typedef typename Expr::Precision Precision;
	typedef typename Expr::template VLayout<Dynamic, Precision> Base;
	return Base(rhs, lhs);
}

template<class P1, class B1>
Vector<Dynamic, P1, NegExpr<P1,B1> > operator-(const Vector<Dynamic, P1, B1>& v)
{
	return typename NegExpr<P1,B1>::template VLayout<Dynamic, P1>(v);
}

}


using namespace TooN;
using namespace std;
extern "C"{
void foo(const Vector<>& v1, const Vector<>& v2, Vector<>& v3, int i)
{
	v3 =-3*(v1+2.4*v2).slice(0,i)/2;
}
}


int main()
{
	Vector<> v1 = makeVector(1,2,3);
	Vector<> v2 = makeVector(6,3,1);

	Vector<> v3 = -3*(v1 + 2*v2).slice(0,2) / 2;

	cout << (v1+v2)[0] << endl;
	cout << v3 << endl;
}