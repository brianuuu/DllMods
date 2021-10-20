#pragma once

template<typename T>
struct SharedPtr
{
    T* value;
    void* implementation; // boost::detail::sp_counted_impl_p<T>
};

struct SynchronizedObject
{
    SharedPtr<CRITICAL_SECTION>* criticalSection;
    void* object;

    class Lock
    {
    protected:
        SynchronizedObject* synchronizedObject;
        bool success;

    public:
        Lock(SynchronizedObject* synchronizedObject);
        ~Lock();

        void* getObject() const;
        bool getSuccess() const;
    };
};