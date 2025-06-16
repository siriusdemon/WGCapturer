#pragma once
#include "pch.h"

#define DLL_API __declspec(dllexport)

#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.directx.direct3d11.interop.h> 

#include <opencv2/opencv.hpp>
// helper to create item
inline auto CreateCaptureItemForWindow(HWND hwnd)
{
    auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };
    winrt::check_hresult(interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item)));
    return item;
}

inline auto CreateD2DDevice(winrt::com_ptr<ID2D1Factory1> const& factory, winrt::com_ptr<ID3D11Device> const& device)
{
    winrt::com_ptr<ID2D1Device> result;
    winrt::check_hresult(factory->CreateDevice(device.as<IDXGIDevice>().get(), result.put()));
    return result;
}

class DLL_API WGCapturer {
public:
    WGCapturer(HWND hwnd);

    ~WGCapturer() {
        try {
            m_session.Close();
            m_framePool.Close();
        }
        catch (...) {}
    }

    std::vector<cv::Mat> capture(const std::vector<std::tuple<int, int, int, int>>& regions);
    void setTargetRegion(int left, int top, int width, int height);

private:
    HWND hwnd;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ nullptr };
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
    winrt::Windows::Graphics::DirectX::DirectXPixelFormat m_format;
    winrt::com_ptr<ID3D11Device> d3dDevice;

    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };


    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame CaptureFrame();
    cv::Mat frameToCv(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame& frame);

    std::mutex mutex_;
    bool crop;
    int Left, Top, Width, Height;
};





struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
    IDirect3DDxgiInterfaceAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};
template <typename T>
auto GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const& object)
{
    auto access = object.as<IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<T> result;
    winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
    return result;
}


template<typename T>
winrt::com_ptr<T> GetDXGIInterfaceFromSurface(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface const& surface)
{
    winrt::com_ptr<T> result;
    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    winrt::check_hresult(access->GetInterface(__uuidof(T), result.put_void()));
    return result;

}

inline auto CreateD3DDevice()
{
    winrt::com_ptr<ID3D11Device> d3dDevice;
    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    winrt::com_ptr<IInspectable> inspectable;

    D3D_FEATURE_LEVEL fl;
    winrt::check_hresult(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        d3dDevice.put(),
        &fl,
        nullptr));

    dxgiDevice = d3dDevice.as<IDXGIDevice>();
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put()));

    return std::make_pair(inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>(), d3dDevice);
}


inline auto CreateD3D11Device(D3D_DRIVER_TYPE const type, UINT flags, winrt::com_ptr<ID3D11Device>& device)
{
    WINRT_ASSERT(!device);

    return D3D11CreateDevice(nullptr, type, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, device.put(),
        nullptr, nullptr);
}


inline auto CreateD3D11Device(UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT)
{
    winrt::com_ptr<ID3D11Device> device;
    HRESULT hr = CreateD3D11Device(D3D_DRIVER_TYPE_HARDWARE, flags, device);
    if (DXGI_ERROR_UNSUPPORTED == hr)
    {
        hr = CreateD3D11Device(D3D_DRIVER_TYPE_WARP, flags, device);
    }

    winrt::check_hresult(hr);
    return device;
}


inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device)
{
    winrt::com_ptr<::IInspectable> d3d_device;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
    return d3d_device.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}
