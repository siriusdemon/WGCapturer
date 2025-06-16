#include <wincodec.h> // WIC


#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "windowscodecs.lib")


//
//bool SaveTextureToPNG(ID3D11Device* d3dDevice, ID3D11Texture2D* texture, const std::wstring& filename)
//{
//    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
//    d3dDevice->GetImmediateContext(&context);
//
//     获取原纹理信息
//    D3D11_TEXTURE2D_DESC desc = {};
//    texture->GetDesc(&desc);
//
//     创建可CPU读取的 staging texture
//    D3D11_TEXTURE2D_DESC stagingDesc = desc;
//    stagingDesc.Usage = D3D11_USAGE_STAGING;
//    stagingDesc.BindFlags = 0;
//    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//    stagingDesc.MiscFlags = 0;
//
//    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTex;
//    HRESULT hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
//    if (FAILED(hr)) return false;
//
//     拷贝资源到 staging
//    context->CopyResource(stagingTex.Get(), texture);
//
//     映射读取像素
//    D3D11_MAPPED_SUBRESOURCE mapped;
//    hr = context->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
//    if (FAILED(hr)) return false;
//
//     初始化 WIC
//    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
//    hr = CoInitialize(nullptr); // 如果还没初始化 COM
//    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
//        IID_PPV_ARGS(&wicFactory));
//    if (FAILED(hr)) return false;
//
//     创建 WIC 位图
//    Microsoft::WRL::ComPtr<IWICBitmap> wicBitmap;
//    hr = wicFactory->CreateBitmapFromMemory(
//        desc.Width, desc.Height, GUID_WICPixelFormat32bppBGRA,
//        mapped.RowPitch,
//        mapped.RowPitch * desc.Height,
//        static_cast<BYTE*>(mapped.pData),
//        &wicBitmap);
//    context->Unmap(stagingTex.Get(), 0);
//
//    if (FAILED(hr)) return false;
//
//     创建流
//    Microsoft::WRL::ComPtr<IWICStream> stream;
//    hr = wicFactory->CreateStream(&stream);
//    if (FAILED(hr)) return false;
//
//    hr = stream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
//    if (FAILED(hr)) return false;
//
//     创建编码器
//    Microsoft::WRL::ComPtr<IWICBitmapEncoder> encoder;
//    hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
//    if (FAILED(hr)) return false;
//
//    hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
//    if (FAILED(hr)) return false;
//
//     创建帧并写入
//    Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> frame;
//    Microsoft::WRL::ComPtr<IPropertyBag2> props;
//    hr = encoder->CreateNewFrame(&frame, &props);
//    if (FAILED(hr)) return false;
//
//    hr = frame->Initialize(props.Get());
//    hr = frame->SetSize(desc.Width, desc.Height);
//    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
//    hr = frame->SetPixelFormat(&format);
//    hr = frame->WriteSource(wicBitmap.Get(), nullptr);
//    hr = frame->Commit();
//    hr = encoder->Commit();
//
//    return SUCCEEDED(hr);
//}


#include <opencv2/opencv.hpp>

bool SaveTextureToPNG(ID3D11Device* d3dDevice, ID3D11Texture2D* texture, const std::string& filename)
{
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    d3dDevice->GetImmediateContext(&context);

    // 获取纹理描述
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    // 创建 CPU 可访问的 staging texture
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTex;
    HRESULT hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
    if (FAILED(hr)) return false;

    // 复制资源
    context->CopyResource(stagingTex.Get(), texture);

    // 映射读取像素数据
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = context->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return false;

    // 将数据复制到 OpenCV 的 Mat（BGRA）
    int width = desc.Width;
    int height = desc.Height;
    int rowPitch = mapped.RowPitch;

    // 注意：OpenCV 中 BGRA 是 CV_8UC4
    cv::Mat bgraImage(height, width, CV_8UC4);
    BYTE* srcData = static_cast<BYTE*>(mapped.pData);

    for (int y = 0; y < height; ++y) {
        memcpy(bgraImage.ptr(y), srcData + y * rowPitch, width * 4); // 4 字节 = BGRA
    }

    context->Unmap(stagingTex.Get(), 0);

    // 转换为 BGR（去掉 Alpha）
    cv::Mat bgrImage;
    cv::cvtColor(bgraImage, bgrImage, cv::COLOR_BGRA2BGR);
    //cv::Rect roi(1, 31, width - 2, height - 32);    // remove the outer
    //cv::Mat cropped = bgrImage(roi);

    // 保存为 PNG
    return cv::imwrite(filename, bgrImage);
}

cv::Mat Texture2Mat(ID3D11Device* d3dDevice, ID3D11Texture2D* texture) {
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    d3dDevice->GetImmediateContext(&context);

    // 获取纹理描述
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    // 创建 CPU 可访问的 staging texture
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTex;
    HRESULT hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
    if (FAILED(hr)) return cv::Mat();

    // 复制资源
    context->CopyResource(stagingTex.Get(), texture);

    // 映射读取像素数据
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = context->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return cv::Mat();

    // 将数据复制到 OpenCV 的 Mat（BGRA）
    int width = desc.Width;
    int height = desc.Height;
    int rowPitch = mapped.RowPitch;

    // 注意：OpenCV 中 BGRA 是 CV_8UC4
    cv::Mat bgraImage(height, width, CV_8UC4);
    BYTE* srcData = static_cast<BYTE*>(mapped.pData);

    for (int y = 0; y < height; ++y) {
        memcpy(bgraImage.ptr(y), srcData + y * rowPitch, width * 4); // 4 字节 = BGRA
    }

    context->Unmap(stagingTex.Get(), 0);

    // 转换为 BGR（去掉 Alpha）
    cv::Mat bgrImage;
    cv::cvtColor(bgraImage, bgrImage, cv::COLOR_BGRA2BGR);
    return bgrImage.clone();
}
