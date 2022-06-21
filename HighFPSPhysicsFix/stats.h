#pragma once

namespace SDT
{
    struct StatsCounter
    {
    public:
        StatsCounter()
        {
            reset();
        }

        __forceinline void reset()
        {
            num = 0;
            fval = 0.0;
            s = IPerfCounter::Query();
        }

        __forceinline bool update(long long m, long long& out)
        {
            auto e = IPerfCounter::Query();
            auto delta = IPerfCounter::delta_us(s, e);

            num++;

            if (delta < m || num == 0) {
                return false;
            }

            out = delta / num;

            num = 0;
            s = e;

            return true;
        }

        __forceinline void accum(double val)
        {
            num++;
            fval += val;
        }

        __forceinline bool get(double& out)
        {
            if (!num) {
                return false;
            }

            out = fval / static_cast<double>(num);

            num = 0;
            fval = 0.0;

            return true;
        }

    private:
        long long s;

        double fval;
        uint64_t num;
    };

}