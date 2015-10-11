// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
#include "fixed.hpp"

__int64 const internal_pi=0x3243f6a8;
__int64 const internal_two_pi=0x6487ed51;
__int64 const internal_half_pi=0x1921fb54;
__int64 const internal_quarter_pi=0xc90fdaa;

extern fixed const fixed_pi(fixed::internal(),internal_pi);
extern fixed const fixed_two_pi(fixed::internal(),internal_two_pi);
extern fixed const fixed_half_pi(fixed::internal(),internal_half_pi);
extern fixed const fixed_quarter_pi(fixed::internal(),internal_quarter_pi);

fixed& fixed::operator%=(fixed const& other)
{
    m_nVal = m_nVal%other.m_nVal;
    return *this;
}

fixed& fixed::operator*=(fixed const& val)
{
    bool const val_negative=val.m_nVal<0;
    bool const this_negative=m_nVal<0;
    bool const negate=val_negative ^ this_negative;
    unsigned __int64 const other=val_negative?-val.m_nVal:val.m_nVal;
    unsigned __int64 const self=this_negative?-m_nVal:m_nVal;
    
    if(unsigned __int64 const self_upper=(self>>32))
    {
        m_nVal=(self_upper*other)<<(32-fixed_resolution_shift);
    }
    else
    {
        m_nVal=0;
    }
    if(unsigned __int64 const self_lower=(self&0xffffffff))
    {
        unsigned long const other_upper=static_cast<unsigned long>(other>>32);
        unsigned long const other_lower=static_cast<unsigned long>(other&0xffffffff);
        unsigned __int64 const lower_self_upper_other_res=self_lower*other_upper;
        unsigned __int64 const lower_self_lower_other_res=self_lower*other_lower;
        m_nVal+=(lower_self_upper_other_res<<(32-fixed_resolution_shift))
            + (lower_self_lower_other_res>>fixed_resolution_shift);
    }
    
    if(negate)
    {
        m_nVal=-m_nVal;
    }
    return *this;
}


fixed& fixed::operator/=(fixed const& divisor)
{
    if( !divisor.m_nVal)
    {
        m_nVal=fixed_max.m_nVal;
    }
    else
    {
        bool const negate_this=(m_nVal<0);
        bool const negate_divisor=(divisor.m_nVal<0);
        bool const negate=negate_this ^ negate_divisor;
        unsigned __int64 a=negate_this?-m_nVal:m_nVal;
        unsigned __int64 b=negate_divisor?-divisor.m_nVal:divisor.m_nVal;

        unsigned __int64 res=0;
    
        unsigned __int64 temp=b;
        bool const a_large=a>b;
        unsigned shift=fixed_resolution_shift;

        if(a_large)
        {
            unsigned __int64 const half_a=a>>1;
            while(temp<half_a)
            {
                temp<<=1;
                ++shift;
            }
        }
        unsigned __int64 d=1I64<<shift;
        if(a_large)
        {
            a-=temp;
            res+=d;
        }

        while(a && temp && shift)
        {
            unsigned right_shift=0;
            while(right_shift<shift && (temp>a))
            {
                temp>>=1;
                ++right_shift;
            }
            d>>=right_shift;
            shift-=right_shift;
            a-=temp;
            res+=d;
        }
        m_nVal=(negate?-(__int64)res:res);
    }
    
    return *this;
}


fixed fixed::sqrt() const
{
    unsigned const max_shift=62;
    unsigned __int64 a_squared=1I64<<max_shift;
    unsigned b_shift=(max_shift+fixed_resolution_shift)/2;
    unsigned __int64 a=1I64<<b_shift;
    
    unsigned __int64 x=m_nVal;
    
    while(b_shift && a_squared>x)
    {
        a>>=1;
        a_squared>>=2;
        --b_shift;
    }

    unsigned __int64 remainder=x-a_squared;
    --b_shift;
    
    while(remainder && b_shift)
    {
        unsigned __int64 b_squared=1I64<<(2*b_shift-fixed_resolution_shift);
        int const two_a_b_shift=b_shift+1-fixed_resolution_shift;
        unsigned __int64 two_a_b=(two_a_b_shift>0)?(a<<two_a_b_shift):(a>>-two_a_b_shift);
        
        while(b_shift && remainder<(b_squared+two_a_b))
        {
            b_squared>>=2;
            two_a_b>>=1;
            --b_shift;
        }
        unsigned __int64 const delta=b_squared+two_a_b;
        if((2*remainder)>delta)
        {
            a+=(1I64<<b_shift);
            remainder-=delta;
            if(b_shift)
            {
                --b_shift;
            }
        }
    }
    return fixed(internal(),a);
}

namespace
{
    int const max_power=63-fixed_resolution_shift;
    __int64 const log_two_power_n_reversed[]={
        0x18429946EI64,0x1791272EFI64,0x16DFB516FI64,0x162E42FF0I64,0x157CD0E70I64,0x14CB5ECF1I64,0x1419ECB71I64,0x13687A9F2I64,
        0x12B708872I64,0x1205966F3I64,0x115424573I64,0x10A2B23F4I64,0xFF140274I64,0xF3FCE0F5I64,0xE8E5BF75I64,0xDDCE9DF6I64,
        0xD2B77C76I64,0xC7A05AF7I64,0xBC893977I64,0xB17217F8I64,0xA65AF679I64,0x9B43D4F9I64,0x902CB379I64,0x851591FaI64,
        0x79FE707bI64,0x6EE74EFbI64,0x63D02D7BI64,0x58B90BFcI64,0x4DA1EA7CI64,0x428AC8FdI64,0x3773A77DI64,0x2C5C85FeI64,
        0x2145647EI64,0x162E42FfI64,0xB17217FI64
    };
    
    __int64 const log_one_plus_two_power_minus_n[]={
        0x67CC8FBI64,0x391FEF9I64,0x1E27077I64,0xF85186I64,
        0x7E0A6CI64,0x3F8151I64,0x1FE02AI64,0xFF805I64,0x7FE01I64,0x3FF80I64,0x1FFE0I64,0xFFF8I64,
        0x7FFEI64,0x4000I64,0x2000I64,0x1000I64,0x800I64,0x400I64,0x200I64,0x100I64,
        0x80I64,0x40I64,0x20I64,0x10I64,0x8I64,0x4I64,0x2I64,0x1I64
    };

    __int64 const log_one_over_one_minus_two_power_minus_n[]={
        0xB172180I64,0x49A5884I64,0x222F1D0I64,0x108598BI64,
        0x820AECI64,0x408159I64,0x20202BI64,0x100805I64,0x80201I64,0x40080I64,0x20020I64,0x10008I64,
        0x8002I64,0x4001I64,0x2000I64,0x1000I64,0x800I64,0x400I64,0x200I64,0x100I64,
        0x80I64,0x40I64,0x20I64,0x10I64,0x8I64,0x4I64,0x2I64,0x1I64
    };
}


fixed fixed::exp() const
{
    if(m_nVal>=log_two_power_n_reversed[0])
    {
        return fixed_max;
    }
    if(m_nVal<-log_two_power_n_reversed[63-2*fixed_resolution_shift])
    {
        return fixed(internal(),0);
    }
    if(!m_nVal)
    {
        return fixed(internal(),fixed_resolution);
    }

    __int64 res=fixed_resolution;

    if(m_nVal>0)
    {
        int power=max_power;
        __int64 const* log_entry=log_two_power_n_reversed;
        __int64 temp=m_nVal;
        while(temp && power>(-(int)fixed_resolution_shift))
        {
            while(!power || (temp<*log_entry))
            {
                if(!power)
                {
                    log_entry=log_one_plus_two_power_minus_n;
                }
                else
                {
                    ++log_entry;
                }
                --power;
            }
            temp-=*log_entry;
            if(power<0)
            {
                res+=(res>>(-power));
            }
            else
            {
                res<<=power;
            }
        }
    }
    else
    {
        int power=fixed_resolution_shift;
        __int64 const* log_entry=log_two_power_n_reversed+(max_power-power);
        __int64 temp=m_nVal;

        while(temp && power>(-(int)fixed_resolution_shift))
        {
            while(!power || (temp>(-*log_entry)))
            {
                if(!power)
                {
                    log_entry=log_one_over_one_minus_two_power_minus_n;
                }
                else
                {
                    ++log_entry;
                }
                --power;
            }
            temp+=*log_entry;
            if(power<0)
            {
                res-=(res>>(-power));
            }
            else
            {
                res>>=power;
            }
        }
    }
    
    return fixed(internal(),res);
}

fixed fixed::log() const
{
    if(m_nVal<=0)
    {
        return -fixed_max;
    }
    if(m_nVal==fixed_resolution)
    {
        return fixed_zero;
    }
    unsigned __int64 temp=m_nVal;
    int left_shift=0;
    unsigned __int64 const scale_position=0x8000000000000000;
    while(temp<scale_position)
    {
        ++left_shift;
        temp<<=1;
    }
    
    __int64 res=(left_shift<max_power)?
        log_two_power_n_reversed[left_shift]:
        -log_two_power_n_reversed[2*max_power-left_shift];
    unsigned right_shift=1;
    unsigned __int64 shifted_temp=temp>>1;
    while(temp && (right_shift<fixed_resolution_shift))
    {
        while((right_shift<fixed_resolution_shift) && (temp<(shifted_temp+scale_position)))
        {
            shifted_temp>>=1;
            ++right_shift;
        }
        
        temp-=shifted_temp;
        shifted_temp=temp>>right_shift;
        res+=log_one_over_one_minus_two_power_minus_n[right_shift-1];
    }
    return fixed(fixed::internal(),res);
}


namespace
{
    const long arctantab[32] = {
        297197971, 210828714, 124459457, 65760959, 33381290, 16755422, 8385879,
        4193963, 2097109, 1048571, 524287, 262144, 131072, 65536, 32768, 16384,
        8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1, 0, 0,
    };


    long scale_cordic_result(long a)
    {
        long const cordic_scale_factor=0x22C2DD1C; /* 0.271572 * 2^31*/
        return (long)((((__int64)a)*cordic_scale_factor)>>31);
    }
    
    long right_shift(long val,int shift)
    {
        return (shift<0)?(val<<-shift):(val>>shift);
    }
    
    void perform_cordic_rotation(long&px, long&py, long theta)
    {
        long x = px, y = py;
        long const *arctanptr = arctantab;
        for (int i = -1; i <= (int)fixed_resolution_shift; ++i)
        {
            long const yshift=right_shift(y,i);
            long const xshift=right_shift(x,i);

            if (theta < 0)
            {
                x += yshift;
                y -= xshift;
                theta += *arctanptr++;
            }
            else
            {
                x -= yshift;
                y += xshift;
                theta -= *arctanptr++;
            }
        }
        px = scale_cordic_result(x);
        py = scale_cordic_result(y);
    }


    void perform_cordic_polarization(long& argx, long&argy)
    {
        long theta=0;
        long x = argx, y = argy;
        long const *arctanptr = arctantab;
        for(int i = -1; i <= (int)fixed_resolution_shift; ++i)
        {
            long const yshift=right_shift(y,i);
            long const xshift=right_shift(x,i);
            if(y < 0)
            {
                y += xshift;
                x -= yshift;
                theta -= *arctanptr++;
            }
            else
            {
                y -= xshift;
                x += yshift;
                theta += *arctanptr++;
            }
        }
        argx = scale_cordic_result(x);
        argy = theta;
    }
}

void fixed::sin_cos(fixed const& theta,fixed* s,fixed*c)
{
    __int64 x=theta.m_nVal%internal_two_pi;
    if( x < 0 )
        x += internal_two_pi;

    bool negate_cos=false;
    bool negate_sin=false;

    if( x > internal_pi )
    {
        x =internal_two_pi-x;
        negate_sin=true;
    }
    if(x>internal_half_pi)
    {
        x=internal_pi-x;
        negate_cos=true;
    }
    long x_cos=1<<28,x_sin=0;

    perform_cordic_rotation(x_cos,x_sin,(long)x);

    if(s)
    {
        s->m_nVal=negate_sin?-x_sin:x_sin;
    }
    if(c)
    {
        c->m_nVal=negate_cos?-x_cos:x_cos;
    }
}

fixed fixed::atan() const
{
    fixed r,theta;
    to_polar(1,*this,&r,&theta);
    return theta;
}

void fixed::to_polar(fixed const& x,fixed const& y,fixed* r,fixed*theta)
{
    bool const negative_x=x.m_nVal<0;
    bool const negative_y=y.m_nVal<0;
    
    unsigned __int64 a=negative_x?-x.m_nVal:x.m_nVal;
    unsigned __int64 b=negative_y?-y.m_nVal:y.m_nVal;

    unsigned right_shift=0;
    unsigned const max_value=1U<<fixed_resolution_shift;

    while((a>=max_value) || (b>=max_value))
    {
        ++right_shift;
        a>>=1;
        b>>=1;
    }
    long xtemp=(long)a;
    long ytemp=(long)b;
    perform_cordic_polarization(xtemp,ytemp);
    r->m_nVal=__int64(xtemp)<<right_shift;
    theta->m_nVal=ytemp;

    if(negative_x && negative_y)
    {
        theta->m_nVal-=internal_pi;
    }
    else if(negative_x)
    {
        theta->m_nVal=internal_pi-theta->m_nVal;
    }
    else if(negative_y)
    {
        theta->m_nVal=-theta->m_nVal;
    }
}

