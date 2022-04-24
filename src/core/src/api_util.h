#pragma once
#define ALBC_IS_INTERNAL
#include "albc/albc_common.h"
#include "albc_types.h"
#include "util_log.h"
#include "util_exception.h"
#include "util_locale.h"

#include <atomic>
#include <cstring>
#include <stdexcept>

//#define ALBC_OBJECT_HANDLE_DEBUG
// uncomment to print debug info on handle creation/destruction

#define CALBC_HANDLE_IMPL(name, type)                                                                                  \
    struct name : public AlbcObjectHandle<type>                                                                        \
    {                                                                                                                  \
        using AlbcObjectHandle<type>::AlbcObjectHandle;                                                                \
    };

AlbcException *ThrowApiException(const std::string &msg);

[[maybe_unused]] void TranslateException(AlbcException **ep, std::string &out_msg);

#define ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(receiver, ...)                                                          \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        std::string msg;                                                                                               \
        TranslateException(receiver, msg);                                                                             \
        LOG_E("Error: ", __VA_ARGS__, ": ", msg);                                                                      \
        if (receiver == nullptr)                                                                                       \
        {                                                                                                              \
            LOG_E("An exception was suppressed: ", msg);                                                               \
        }                                                                                                              \
    }

struct AlbcObjectHandleBase
{
  public:
    virtual void *GetObject() = 0;
    virtual AlbcObjectHandleBase *MakeRef() = 0;
    virtual void SetRef(bool is_ref) = 0;
    virtual bool IsRef() = 0;
    virtual ~AlbcObjectHandleBase() = default;
};

template <typename T> struct ReleaseDeleter
{
  public:
    std::shared_ptr<std::atomic<bool>> released;

    ReleaseDeleter() : released(std::make_shared<std::atomic<bool>>(false))
    {
    }

    void SetReleased(bool val = true)
    {
        released->store(val);
    }

    void operator()(T *ptr)
    {
        if (released->load())
            return;

#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D("Deleting object<", albc::util::TypeName<T>(), ">: ", ptr);
        on_object_handle_destroy(ptr);
#endif

        delete ptr;
    }
};

template <typename T> struct AlbcObjectHandle : public AlbcObjectHandleBase
{
  private:
    template <typename TInit> static std::shared_ptr<T> MakeSharedInternal(TInit &&init)
    {
        return std::shared_ptr<T>(std::forward<TInit>(init), ReleaseDeleter<T>());
    }

    ReleaseDeleter<T> *GetDeleter()
    {
        auto *deleter = std::get_deleter<ReleaseDeleter<T>>(impl);
        if (!deleter)
            throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + ": Deleter not found");
        return deleter;
    }

  public:
    std::shared_ptr<T> impl;
    explicit AlbcObjectHandle(T *impl) : impl(MakeSharedInternal(impl))
    {
#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D(__PRETTY_FUNCTION__, ": ", this,": binding: ", impl);
        on_object_handle_create(impl);
        on_object_handle_create(this);
#endif
    }

    AlbcObjectHandle(std::unique_ptr<T> &&ptr) // NOLINT(google-explicit-constructor)
        : AlbcObjectHandle(ptr.release())
    {
    }

    AlbcObjectHandle(const std::shared_ptr<T> &ptr) // NOLINT(google-explicit-constructor)
        : impl(ptr)
    {
#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D(__PRETTY_FUNCTION__, ": ", this, ": making ref: ", impl.get());
        on_object_handle_create(this);
#endif
    }

    AlbcObjectHandle(const AlbcObjectHandle<T> &other)
        : AlbcObjectHandle(other.impl)
    {
    }

    ~AlbcObjectHandle() override
    {
#ifdef ALBC_OBJECT_HANDLE_DEBUG
        on_object_handle_destroy(this);
#endif
        (void)114514; // suppress default-dtor warning
    }

    void *GetObject() override
    {
        return const_cast<T *>(impl.get());
    }

    AlbcObjectHandle<T> *MakeRef() override
    {
        return new AlbcObjectHandle<T>(impl);
    }

    void SetRef(bool is_ref) override
    {
#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D(__PRETTY_FUNCTION__, ": SetRef: ", impl.get(), " ", is_ref);
        if (is_ref)
            on_object_handle_destroy(impl.get());
        else
            on_object_handle_create(impl.get());
#endif

        auto deleter = GetDeleter();

        if (IsRef() == is_ref)
        {
            LOG_W(albc::util::TypeName<T>(), ": Object is already ", is_ref ? "ref" : "non-ref", ": ", impl.get());
            return;
        }

        if (impl.use_count() > 1)
            throw std::runtime_error(
                albc::util::TypeName<T>() +
                ": object is shared and ref state cannot be modified! use count: " + std::to_string(impl.use_count()));

        deleter->SetReleased(is_ref);
    }

    bool IsRef() override
    {
        return GetDeleter()->released->load();
    }

    T *Unwrap(bool reset = false)
    {
        SetRef(true);
#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D(__PRETTY_FUNCTION__, ": Unwrapping: ", impl.get(), " ", reset);
#endif

        auto p = impl.get();
        if (reset)
            impl.reset();

        return p;
    }

    void Rebind(T *obj)
    {
        if (!IsRef())
            throw std::runtime_error(albc::util::TypeName<T>() + ": cannot rebind non-ref handle.");

#ifdef ALBC_OBJECT_HANDLE_DEBUG
        LOG_D(albc::util::TypeName<T>(), ": Rebind: ", impl.get(), " ", obj);
#endif

        impl.reset(obj);
    }
};