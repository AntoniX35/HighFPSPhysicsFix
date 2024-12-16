#pragma once

#include <random>

class RandomNumberGeneratorBase
{
public:
    explicit RandomNumberGeneratorBase() :
        m_generator(m_device())
    {
    }

    virtual ~RandomNumberGeneratorBase() noexcept = default;

    std::random_device m_device;
    std::mt19937 m_generator;
};

template <class T = float, class = std::enable_if_t<std::is_fundamental_v<T>, void>>
class RandomNumberGenerator :
    public RandomNumberGeneratorBase
{
public:

    using producer_type =
        std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>,
        std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>, void>>;

    explicit RandomNumberGenerator(T a_min, T a_max) :
        RandomNumberGeneratorBase(),
        m_producer(a_min, a_max)
    {
    }

    virtual ~RandomNumberGenerator() noexcept = default;

    virtual T Get()
    {
        return m_producer(m_generator);
    }

protected:
    producer_type m_producer;
};

template <class T = float, class = std::enable_if_t<std::is_fundamental_v<T>, void>>
class RandomNumberGenerator2
{
public:

    using producer_type =
        std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>,
        std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>, void>>;

    explicit RandomNumberGenerator2(const std::shared_ptr<RandomNumberGeneratorBase>& a_base, T a_min, T a_max) :
        m_base(a_base),
        m_producer(a_min, a_max)
    {
    }

    virtual ~RandomNumberGenerator2() noexcept = default;

    virtual T Get()
    {
        return m_producer(m_base->m_generator);
    }

protected:
    std::shared_ptr<RandomNumberGeneratorBase> m_base;
    producer_type m_producer;
};

template <class T = float, class = std::enable_if_t<std::is_fundamental_v<T>, void>>
class RandomNumberGenerator3
{
public:

    using producer_type =
        std::conditional_t<std::is_floating_point_v<T>, std::uniform_real_distribution<T>,
        std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>, void>>;

    explicit RandomNumberGenerator3(T a_min, T a_max) :
        m_producer(a_min, a_max)
    {
    }

    virtual T Get(const RandomNumberGeneratorBase& a_base)
    {
        return m_producer(a_base.m_generator);
    }

protected:
    producer_type m_producer;
};

#include "Threads.h"

template <class T = float>
class ThreadSafeRandomNumberGenerator :
    public RandomNumberGenerator<T>
{
public:

    using RandomNumberGenerator<T>::RandomNumberGenerator;

    virtual T Get()
    {
        IScopedLock _(m_lock);
        return m_producer(RandomNumberGeneratorBase::m_generator);
    }

private:
    FastSpinLock m_lock;
};