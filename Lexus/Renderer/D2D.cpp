#include "D2D.h"
#include <unordered_map>
#include <winrt/base.h>
#include "../Client/Client.h"
#include "../SDK/Game.h"
#include "../Utils/TimerUtil.h"
#include "../Utils/Logger.h"

float InterfaceUtils::deltaTime = 0.016f;

//d2d stuff
static ID2D1Factory3* d2dFactory = nullptr;
static IDWriteFactory* d2dWriteFactory = nullptr;
static ID2D1Device2* d2dDevice = nullptr;
static ID2D1DeviceContext2* d2dDeviceContext = nullptr;
static ID2D1Bitmap1* sourceBitmap = nullptr;
static ID2D1Effect* blurEffect = nullptr;

//cache
static std::unordered_map<float, winrt::com_ptr<IDWriteTextFormat>> textFormatCache;
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutCache;
static std::unordered_map<uint32_t, winrt::com_ptr<ID2D1SolidColorBrush>> colorBrushCache;

//temporary cache
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutTemporary;

static int currentD2DFontSize = 25;
static std::string currentD2DFont = "Arial";
static bool isFontItalic = false;

static bool initD2D = false;

template<typename T>
void SafeRelease(T*& ptr) {
	if (ptr != nullptr) {
		ptr->Release();
		ptr = nullptr;
	}
}

std::wstring to_wide(const std::string& str);
uint64_t getTextLayoutKey(const std::string& textStr, float textSize);
IDWriteTextFormat* getTextFormat(float textSize);
IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize, bool storeTextLayout = true);
ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color);

void InterfaceUtils::NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi) {
	if (!initD2D) {

		D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2dFactory);

		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(d2dWriteFactory), reinterpret_cast<IUnknown**>(&d2dWriteFactory));

		IDXGIDevice* dxgiDevice;
		d3d11Device->QueryInterface<IDXGIDevice>(&dxgiDevice);
		d2dFactory->CreateDevice(dxgiDevice, &d2dDevice);
		dxgiDevice->Release();

		d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);
		//d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &d2dDeviceContext);

		d2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &blurEffect);
		blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
		blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

		IDXGISurface* dxgiBackBuffer = nullptr;
		swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
		D2D1_BITMAP_PROPERTIES1 bitmapProperties
			= D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fxdpi, fxdpi);
		d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties, &sourceBitmap);
		dxgiBackBuffer->Release();
		d2dDeviceContext->SetTarget(sourceBitmap);

		initD2D = true;
	}

	d2dDeviceContext->BeginDraw();
}

void InterfaceUtils::EndFrame() {
	if (!initD2D)
		return;

	d2dDeviceContext->EndDraw();

	static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
	if ((currentD2DFont != customFontMod->getSelectedFont()) || (currentD2DFontSize != customFontMod->fontSize) || (isFontItalic != customFontMod->italic)) {
		currentD2DFont = customFontMod->getSelectedFont();
		currentD2DFontSize = customFontMod->fontSize;
		isFontItalic = customFontMod->italic;
		textFormatCache.clear();
		textLayoutCache.clear();
		//textLayoutTemporary.clear();
	}

	static float timeCounter = 0.0f;
	timeCounter += InterfaceUtils::deltaTime;
	if (timeCounter > 90.f) {
		if (textFormatCache.size() > 1000)
			textFormatCache.clear();

		if (textLayoutCache.size() > 500)
			textLayoutCache.clear();

		if (colorBrushCache.size() > 2000)
			colorBrushCache.clear();

		timeCounter = 0.0f;
	}

	textLayoutTemporary.clear();
}

std::string toLower(std::string s) {
	for (auto& c : s) c = std::tolower(c);
	return s;
}

std::string SanitizeMCText(const std::string& input) {
	std::string output;
	output.reserve(input.size());

	for (size_t i = 0; i < input.size(); ++i) {
		if (input[i] == '§') {
			i++;
		}
		else {
			output += input[i];
		}
	}
	return output;
}

void InterfaceUtils::Render() {
	ModuleManager::InterfaceUtilsHook();
	NotificationManager::Render();

	{
		static ClickGUI* clickGuiMod = ModuleManager::getModule<ClickGUI>();
		if (clickGuiMod->isEnabled())
			clickGuiMod->Render();
	}

	{
		static Console* console = ModuleManager::getModule<Console>();
		static std::string lastLogMessage;

		if (console->isEnabled())
			console->render();

		auto tryLog = [&](const std::string& msg) {
			if (msg != lastLogMessage) {
				console->log(msg);
				lastLogMessage = msg;
			}
			};

		if (!g_Data.getLocalPlayer()) {
			tryLog("Critical: LocalPlayer is nullptr");
		}
		else if (!g_Data.getGameMode()) {
			tryLog("Critical: GameMode is nullptr");
		}
		else if (!g_Data.getClientInstance()) {
			tryLog("Critical: ClientInstance is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getRenderPositionComponent()) {
			tryLog("Warning: getRenderPositionComponent is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getActorEquipmentComponent()) {
			tryLog("Warning: getActorEquipmentComponent is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getAbsorption()) {
			tryLog("Warning: getAbsorption is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getAnimationComponent()) {
			tryLog("Warning: getAnimationComponent is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getActorHeadRotationComponent()) {
			tryLog("Critical: getActorHeadRotationComponent is nullptr");
		}
		else if (!g_Data.getLocalPlayer()->getEntityContext()) {
			tryLog("Critical: getEntityContext is nullptr");
		}
		else {
			lastLogMessage.clear();
		}
	}

	{
		float titleTextSize = 2.0f;
		float subtitleTextSize = 1.0f;
		float creditsTitleSize = 1.0f;
		float creditsEntrySize = 0.85f;
		float margin = 4.0f;

		UIColor color = UIColor(255, 255, 255);

		float textX = margin;
		float titleY = -1;
		float titleHeight = GetTextHeight("X", titleTextSize);
		float subtitleHeight = GetTextHeight("X", subtitleTextSize);
		float subtitleY = titleY + titleHeight;

		if (g_Data.getLocalPlayer() == nullptr) {
			float y = subtitleY + subtitleHeight + 4.f;

			RenderText(Vec2<float>(textX, y), "Credits", color, creditsTitleSize);
			y += GetTextHeight("Credits", creditsTitleSize);

			RenderText(Vec2<float>(textX + 4.f, y), "Made by Karrot", color, creditsEntrySize);
			y += GetTextHeight("Made by Karrot", creditsEntrySize);

			y += 6.f; 

			RenderText(Vec2<float>(textX, y), "Changelog", color, creditsTitleSize);
			y += GetTextHeight("Changelog", creditsTitleSize);

			RenderText(Vec2<float>(textX + 4.f, y), "[+] Release", color, creditsEntrySize);
			y += GetTextHeight("[+] Release", creditsEntrySize);

			Vec2<float> what = g_Data.getClientInstance()->getguiData()->windowSizeReal;

		}
	}

	// Eject
	{
		Vec2<float> windowSize = InterfaceUtils::getWindowSize();
		static float holdTime = 0.f;
		static float holdAnim = 0.f;
		static float showDuration = 0.f;
		static float exitDuration = 0.f;
		static float exitVelocity = 0.f;

		if (showDuration > 0.1f) {
			static std::string text = "Hold Ctrl and L to eject";
			float textSize = 1.f * showDuration;
			float textPaddingX = 4.f * textSize;
			float textPaddingY = 1.f * textSize;
			float textWidth = GetTextWidth(text, textSize);
			float textHeight = GetTextHeight(text, textSize);

			Vec2<float> textPos = Vec2<float>((windowSize.x - textWidth) / 2.f, -30.f + (100.f * showDuration));
			Vec4<float> rectPos = Vec4<float>(textPos.x - textPaddingX, textPos.y - textPaddingY, textPos.x + textWidth + textPaddingX, textPos.y + textHeight + textPaddingY);
			Vec4<float> underlineRect = Vec4<float>(rectPos.x, rectPos.w, rectPos.z, rectPos.w + 2.f * textSize);
			Vec4<float> underlineAnim = Vec4<float>(underlineRect.x, underlineRect.y, underlineRect.x + (underlineRect.z - underlineRect.x) * holdAnim, underlineRect.w);

			FillRect(rectPos, UIColor(0, 0, 0, 125));
			FillRect(underlineRect, UIColor(0, 0, 0, 165));
			FillRect(underlineAnim, UIColor(255, 255, 255));
			RenderText(textPos, text, UIColor(255, 255, 255), textSize);
		}

		if (g_Data.isKeyDown(VK_CONTROL) && g_Data.isKeyDown('L')) {
			holdTime += InterfaceUtils::deltaTime / 2.f;
			if (holdTime > 1.f)
				holdTime = 1.f;
			exitDuration = 1.5f;
		}
		else {
			holdTime = 0.f;
			exitDuration -= InterfaceUtils::deltaTime;
		}

		holdAnim += (holdTime - holdAnim) * (InterfaceUtils::deltaTime * 10.f);
		if (holdAnim > 1.f)
			holdAnim = 1.f;
		if (holdAnim < 0.f)
			holdAnim = 0.f;

		if (exitDuration > 0.f) {
			showDuration += (1.f - showDuration) * (InterfaceUtils::deltaTime * 8.f);
			exitVelocity = 0.f;
		}
		else {
			showDuration -= exitVelocity;
			exitVelocity += InterfaceUtils::deltaTime / 4.f;
		}

		if (showDuration < 0.f)
			showDuration = 0.f;
		if (showDuration > 1.f)
			showDuration = 1.f;

		if (holdAnim > 0.99f)
			Client::shutdown();
	}

}

void InterfaceUtils::Clean() {
	if (!initD2D)
		return;

	SafeRelease(d2dFactory);
	SafeRelease(d2dWriteFactory);
	SafeRelease(d2dDevice);
	SafeRelease(d2dDeviceContext);
	SafeRelease(sourceBitmap);
	SafeRelease(blurEffect);

	textFormatCache.clear();
	textLayoutCache.clear();
	colorBrushCache.clear();

	textLayoutTemporary.clear();

	initD2D = false;
}

void InterfaceUtils::Flush() {
	d2dDeviceContext->Flush();
}

Vec2<float> InterfaceUtils::getWindowSize() {
	D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
	return Vec2<float>((float)size.width, (float)size.height);
}

void InterfaceUtils::RenderText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color, float textSize, bool storeTextLayout) {
	IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);

	static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
	if (customFontMod->shadow) {
		ID2D1SolidColorBrush* shadowColorBrush = getSolidColorBrush(UIColor(0, 0, 0, color.a));
		d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x + 1.f, textPos.y + 1.f), textLayout, shadowColorBrush);
	}

	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout, colorBrush);
}

float InterfaceUtils::GetTextWidth(const std::string& textStr, float textSize, bool storeTextLayout) {
	IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
	DWRITE_TEXT_METRICS textMetrics;
	textLayout->GetMetrics(&textMetrics);

	return textMetrics.widthIncludingTrailingWhitespace;
}

float InterfaceUtils::GetTextHeight(const std::string& textStr, float textSize, bool storeTextLayout) {
	IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
	DWRITE_TEXT_METRICS textMetrics;
	textLayout->GetMetrics(&textMetrics);

	return std::ceilf(textMetrics.height);
}

void InterfaceUtils::drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color, float width) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	d2dDeviceContext->DrawLine(D2D1::Point2F(startPos.x, startPos.y), D2D1::Point2F(endPos.x, endPos.y), colorBrush, width);
}

void InterfaceUtils::DrawRect(const Vec4<float>& rect, const UIColor& color, float width, float rounding, CornerRoundType roundType) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);

	if (rounding > 0.0f && roundType != CornerRoundType::None) {
		D2D1_ROUNDED_RECT roundedRect = { D2D1::RectF(rect.x, rect.y, rect.z, rect.w), 0.0f, 0.0f };

		switch (roundType) {
		case CornerRoundType::Full:
			roundedRect.radiusX = rounding;
			roundedRect.radiusY = rounding;
			d2dDeviceContext->DrawRoundedRectangle(&roundedRect, colorBrush, width);
			break;

		case CornerRoundType::TopOnly:
		case CornerRoundType::BottomOnly:
		case CornerRoundType::SidesOnly:
		case CornerRoundType::Left:
		case CornerRoundType::Right: {
			ID2D1PathGeometry* geometry = nullptr;
			ID2D1GeometrySink* sink = nullptr;
			d2dFactory->CreatePathGeometry(&geometry);
			geometry->Open(&sink);

			if (sink) {
				bool top = roundType == CornerRoundType::TopOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool bottom = roundType == CornerRoundType::BottomOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool left = roundType == CornerRoundType::Left || roundType == CornerRoundType::SidesOnly;
				bool right = roundType == CornerRoundType::Right || roundType == CornerRoundType::SidesOnly;

				sink->BeginFigure(D2D1::Point2F(rect.x, rect.y + (top && left ? rounding : 0.0f)), D2D1_FIGURE_BEGIN_HOLLOW);

				if (top && left) {
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.x + rounding, rect.y),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.x, rect.y));
				}

				if (top && right) {
					sink->AddLine(D2D1::Point2F(rect.z - rounding, rect.y));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.z, rect.y + rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.z, rect.y));
				}

				if (bottom && right) {
					sink->AddLine(D2D1::Point2F(rect.z, rect.w - rounding));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.z - rounding, rect.w),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.z, rect.w));
				}

				if (bottom && left) {
					sink->AddLine(D2D1::Point2F(rect.x + rounding, rect.w));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.x, rect.w - rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.x, rect.w));
				}

				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();
				sink->Release();
			}

			d2dDeviceContext->DrawGeometry(geometry, colorBrush, width);
			geometry->Release();
			break;
		}

		default:
			d2dDeviceContext->DrawRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush, width);
			break;
		}
	}
	else {
		d2dDeviceContext->DrawRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush, width);
	}
}

void InterfaceUtils::FillGradientRect(const Vec4<float>& rect, const UIColor& color1, const UIColor& color2, float rounding, CornerRoundType roundType) {
	ID2D1GradientStopCollection* stopCollection = nullptr;
	ID2D1LinearGradientBrush* gradientBrush = nullptr;

	D2D1_GRADIENT_STOP stops[2] = {
	  { 0.0f, D2D1::ColorF(color1.r / 255.f, color1.g / 255.f, color1.b / 255.f, color1.a / 255.f) },
	  { 1.0f, D2D1::ColorF(color2.r / 255.f, color2.g / 255.f, color2.b / 255.f, color2.a / 255.f) }
	};

	d2dDeviceContext->CreateGradientStopCollection(stops, 2, &stopCollection);

	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props = {};
	props.startPoint = D2D1::Point2F(rect.x, rect.y);
	props.endPoint = D2D1::Point2F(rect.z, rect.w);

	d2dDeviceContext->CreateLinearGradientBrush(props, stopCollection, &gradientBrush);
	stopCollection->Release();

	if (rounding > 0.0f && roundType != CornerRoundType::None) {
		if (roundType == CornerRoundType::Full) {
			D2D1_ROUNDED_RECT roundedRect = { D2D1::RectF(rect.x, rect.y, rect.z, rect.w), rounding, rounding };
			d2dDeviceContext->FillRoundedRectangle(&roundedRect, gradientBrush);
		}
		else {
			ID2D1PathGeometry* geometry = nullptr;
			ID2D1GeometrySink* sink = nullptr;
			d2dFactory->CreatePathGeometry(&geometry);
			geometry->Open(&sink);

			if (sink) {
				bool top = roundType == CornerRoundType::TopOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool bottom = roundType == CornerRoundType::BottomOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool left = roundType == CornerRoundType::Left || roundType == CornerRoundType::SidesOnly;
				bool right = roundType == CornerRoundType::Right || roundType == CornerRoundType::SidesOnly;

				sink->BeginFigure(D2D1::Point2F(rect.x, rect.y + (top && left ? rounding : 0.0f)), D2D1_FIGURE_BEGIN_FILLED);

				if (top && left) {
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.x + rounding, rect.y),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.x, rect.y));
				}

				if (top && right) {
					sink->AddLine(D2D1::Point2F(rect.z - rounding, rect.y));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.z, rect.y + rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.z, rect.y));
				}

				if (bottom && right) {
					sink->AddLine(D2D1::Point2F(rect.z, rect.w - rounding));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.z - rounding, rect.w),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.z, rect.w));
				}

				if (bottom && left) {
					sink->AddLine(D2D1::Point2F(rect.x + rounding, rect.w));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(rect.x, rect.w - rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(rect.x, rect.w));
				}

				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();
				sink->Release();
			}

			d2dDeviceContext->FillGeometry(geometry, gradientBrush);
			geometry->Release();
		}
	}
	else {
		d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), gradientBrush);
	}

	gradientBrush->Release();
}

void InterfaceUtils::FillRect(const Vec4<float>& rect, const UIColor& color, float rounding, CornerRoundType roundType) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);

	if (rounding > 0.0f && roundType != CornerRoundType::None) {
		D2D1_ROUNDED_RECT roundedRect = { D2D1::RectF(rect.x, rect.y, rect.z, rect.w), 0.0f, 0.0f };

		switch (roundType) {
		case CornerRoundType::Full:
			roundedRect.radiusX = rounding;
			roundedRect.radiusY = rounding;
			d2dDeviceContext->FillRoundedRectangle(&roundedRect, colorBrush);
			break;

		case CornerRoundType::TopOnly:
		case CornerRoundType::BottomOnly:
		case CornerRoundType::SidesOnly:
		case CornerRoundType::Left:
		case CornerRoundType::Right: {
			ID2D1PathGeometry* geometry = nullptr;
			ID2D1GeometrySink* sink = nullptr;
			d2dFactory->CreatePathGeometry(&geometry);
			geometry->Open(&sink);

			if (sink) {
				bool top = roundType == CornerRoundType::TopOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool bottom = roundType == CornerRoundType::BottomOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
				bool left = roundType == CornerRoundType::Left || roundType == CornerRoundType::SidesOnly;
				bool right = roundType == CornerRoundType::Right || roundType == CornerRoundType::SidesOnly;

				float x1 = rect.x;
				float y1 = rect.y;
				float x2 = rect.z;
				float y2 = rect.w;

				sink->BeginFigure(D2D1::Point2F(x1 + (left && top ? rounding : 0.0f), y1), D2D1_FIGURE_BEGIN_FILLED);

				if (top && right) {
					sink->AddLine(D2D1::Point2F(x2 - rounding, y1));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(x2, y1 + rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(x2, y1));
				}

				if (bottom && right) {
					sink->AddLine(D2D1::Point2F(x2, y2 - rounding));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(x2 - rounding, y2),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(x2, y2));
				}

				if (bottom && left) {
					sink->AddLine(D2D1::Point2F(x1 + rounding, y2));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(x1, y2 - rounding),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(x1, y2));
				}

				if (top && left) {
					sink->AddLine(D2D1::Point2F(x1, y1 + rounding));
					sink->AddArc(D2D1::ArcSegment(
						D2D1::Point2F(x1 + rounding, y1),
						D2D1::SizeF(rounding, rounding), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
				}
				else {
					sink->AddLine(D2D1::Point2F(x1, y1));
				}

				sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				sink->Close();
				sink->Release();
			}

			d2dDeviceContext->FillGeometry(geometry, colorBrush);
			geometry->Release();
			break;
		}

		default:
			d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
			break;
		}
	}
	else {
		d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
	}
}

void InterfaceUtils::drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius, float width) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	d2dDeviceContext->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush, width);
}

void InterfaceUtils::fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	d2dDeviceContext->FillEllipse(D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush);
}

void InterfaceUtils::drawTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const UIColor& color, float strokeWidth) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	d2dDeviceContext->DrawLine(D2D1::Point2F(p1.x, p1.y), D2D1::Point2F(p2.x, p2.y), colorBrush, strokeWidth);
	d2dDeviceContext->DrawLine(D2D1::Point2F(p2.x, p2.y), D2D1::Point2F(p3.x, p3.y), colorBrush, strokeWidth);
}

void InterfaceUtils::fillTriangle(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, const UIColor& color) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);

	ID2D1PathGeometry* geometry = nullptr;
	ID2D1GeometrySink* sink = nullptr;

	HRESULT hr = d2dFactory->CreatePathGeometry(&geometry);
	if (SUCCEEDED(hr)) {
		hr = geometry->Open(&sink);
		if (SUCCEEDED(hr)) {
			sink->BeginFigure(D2D1::Point2F(p1.x, p1.y), D2D1_FIGURE_BEGIN_FILLED);
			sink->AddLine(D2D1::Point2F(p2.x, p2.y));
			sink->AddLine(D2D1::Point2F(p3.x, p3.y));
			sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			sink->Close();
			sink->Release();
		}
		d2dDeviceContext->FillGeometry(geometry, colorBrush);
		geometry->Release();
	}
}

void InterfaceUtils::addBlur(const Vec4<float>& rect, float strength, bool flush, CornerRoundType roundType, float rounding) {
	if (flush) d2dDeviceContext->Flush();

	ID2D1Bitmap* targetBitmap = nullptr;
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(sourceBitmap->GetPixelFormat());
	d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), props, &targetBitmap);
	D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
	D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
	D2D1_RECT_U rectU = D2D1::RectU(0, 0, size.width, size.height);
	targetBitmap->CopyFromBitmap(&destPoint, sourceBitmap, &rectU);

	blurEffect->SetInput(0, targetBitmap);
	blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, strength);
	ID2D1Image* outImage = nullptr;
	blurEffect->GetOutput(&outImage);

	ID2D1ImageBrush* brush = nullptr;
	D2D1_RECT_F screenRectF = D2D1::RectF(0.f, 0.f, (float)size.width, (float)size.height);
	D2D1_IMAGE_BRUSH_PROPERTIES brushProps = D2D1::ImageBrushProperties(screenRectF);
	d2dDeviceContext->CreateImageBrush(outImage, brushProps, &brush);

	if (rounding > 0.f && roundType != CornerRoundType::None) {
		if (roundType == CornerRoundType::Full) {
			D2D1_ROUNDED_RECT rRect = { D2D1::RectF(rect.x, rect.y, rect.z, rect.w), rounding, rounding };
			d2dDeviceContext->FillRoundedRectangle(&rRect, brush);
		} else {
			ID2D1PathGeometry* geometry = nullptr;
			ID2D1GeometrySink* sink = nullptr;
			d2dFactory->CreatePathGeometry(&geometry);
			geometry->Open(&sink);

			bool top = roundType == CornerRoundType::TopOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
			bool bottom = roundType == CornerRoundType::BottomOnly || roundType == CornerRoundType::Left || roundType == CornerRoundType::Right;
			bool left = roundType == CornerRoundType::Left || roundType == CornerRoundType::SidesOnly;
			bool right = roundType == CornerRoundType::Right || roundType == CornerRoundType::SidesOnly;

			sink->BeginFigure(D2D1::Point2F(rect.x, rect.y + (top && left ? rounding : 0.f)), D2D1_FIGURE_BEGIN_FILLED);

			if (top && left) sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(rect.x + rounding, rect.y), D2D1::SizeF(rounding, rounding), 0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			else sink->AddLine(D2D1::Point2F(rect.x, rect.y));

			if (top && right) { sink->AddLine(D2D1::Point2F(rect.z - rounding, rect.y)); sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(rect.z, rect.y + rounding), D2D1::SizeF(rounding, rounding), 0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); }
			else sink->AddLine(D2D1::Point2F(rect.z, rect.y));

			if (bottom && right) { sink->AddLine(D2D1::Point2F(rect.z, rect.w - rounding)); sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(rect.z - rounding, rect.w), D2D1::SizeF(rounding, rounding), 0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); }
			else sink->AddLine(D2D1::Point2F(rect.z, rect.w));

			if (bottom && left) { sink->AddLine(D2D1::Point2F(rect.x + rounding, rect.w)); sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(rect.x, rect.w - rounding), D2D1::SizeF(rounding, rounding), 0.f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); }
			else sink->AddLine(D2D1::Point2F(rect.x, rect.w));

			sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			sink->Close();
			sink->Release();

			d2dDeviceContext->FillGeometry(geometry, brush);
			geometry->Release();
		}
	} else {
		ID2D1RectangleGeometry* rectGeo = nullptr;
		d2dFactory->CreateRectangleGeometry(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), &rectGeo);
		d2dDeviceContext->FillGeometry(rectGeo, brush);
		rectGeo->Release();
	}

	targetBitmap->Release();
	outImage->Release();
	brush->Release();
}

void InterfaceUtils::lookAt(float viewMatrix[4][4], const Vec3<float>& eye, const Vec3<float>& center, const Vec3<float>& up) {
	Vec3<float> f = center.sub(eye).normalize();
	Vec3<float> s = f.cross(up).normalize();
	Vec3<float> u = s.cross(f);

	viewMatrix[0][0] = s.x;
	viewMatrix[1][0] = s.y;
	viewMatrix[2][0] = s.z;
	viewMatrix[3][0] = -s.dot(eye);

	viewMatrix[0][1] = u.x;
	viewMatrix[1][1] = u.y;
	viewMatrix[2][1] = u.z;
	viewMatrix[3][1] = -u.dot(eye);

	viewMatrix[0][2] = -f.x;
	viewMatrix[1][2] = -f.y;
	viewMatrix[2][2] = -f.z;
	viewMatrix[3][2] = f.dot(eye);

	viewMatrix[0][3] = 0.f;
	viewMatrix[1][3] = 0.f;
	viewMatrix[2][3] = 0.f;
	viewMatrix[3][3] = 1.f;
}

void InterfaceUtils::perspective(float projectionMatrix[4][4], float fov, float aspect, float zNear, float zFar) {
	float tanHalfFov = tanf(fov / 2.f);
	projectionMatrix[0][0] = 1.f / (aspect * tanHalfFov);
	projectionMatrix[1][0] = 0.f;
	projectionMatrix[2][0] = 0.f;
	projectionMatrix[3][0] = 0.f;

	projectionMatrix[0][1] = 0.f;
	projectionMatrix[1][1] = 1.f / tanHalfFov;
	projectionMatrix[2][1] = 0.f;
	projectionMatrix[3][1] = 0.f;

	projectionMatrix[0][2] = 0.f;
	projectionMatrix[1][2] = 0.f;
	projectionMatrix[2][2] = zFar / (zFar - zNear);
	projectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);

	projectionMatrix[0][3] = 0.f;
	projectionMatrix[1][3] = 0.f;
	projectionMatrix[2][3] = 1.f;
	projectionMatrix[3][3] = 0.f;
}
std::wstring to_wide(const std::string& str) {
	if (str.empty()) return std::wstring();

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);

	return wstr;
}

uint64_t getTextLayoutKey(const std::string& textStr, float textSize) {
	std::hash<std::string> textHash;
	std::hash<float> textSizeHash;
	uint64_t combinedHash = textHash(textStr) ^ textSizeHash(textSize);
	return combinedHash;
}
IDWriteTextFormat* getTextFormat(float textSize) {
	if (textFormatCache[textSize].get() == nullptr) {
		std::wstring fontNameWide = to_wide(currentD2DFont);
		if (fontNameWide.empty()) fontNameWide = L"Microsoft YaHei"; // Ensure Chinese font fallback

		d2dWriteFactory->CreateTextFormat(
			fontNameWide.c_str(),
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			isFontItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			(float)currentD2DFontSize * textSize,
			L"zh-cn", // Use Chinese locale
			textFormatCache[textSize].put()
		);
	}
	return textFormatCache[textSize].get();
}

IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize, bool storeTextLayout) {
	std::wstring wideText = to_wide(textStr);
	IDWriteTextFormat* textFormat = getTextFormat(textSize);
	uint64_t textLayoutKey = getTextLayoutKey(textStr, textSize);

	auto& cache = storeTextLayout ? textLayoutCache : textLayoutTemporary;
	if (cache[textLayoutKey].get() == nullptr) {
		d2dWriteFactory->CreateTextLayout(
			wideText.c_str(),
			(UINT32)wideText.length(),
			textFormat,
			FLT_MAX,
			0.f,
			cache[textLayoutKey].put()
		);
	}
	return cache[textLayoutKey].get();
}

ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color) {
	uint32_t colorBrushKey = ColorUtil::ColorToUInt(color);
	if (colorBrushCache[colorBrushKey].get() == nullptr) {
		d2dDeviceContext->CreateSolidColorBrush(color.toD2D1Color(), colorBrushCache[colorBrushKey].put());
	}
	return colorBrushCache[colorBrushKey].get();
}

void InterfaceUtils::fillRoundingRectangle(const Vec4<float>& rect, const UIColor& color, float radius) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), radius, radius);
	d2dDeviceContext->FillRoundedRectangle(&roundedRect, colorBrush);
}

void InterfaceUtils::drawBox(const AABB& box, const UIColor& fillColor, const UIColor& outlineColor, float thickness, bool fill, bool outline) {
	if (!g_Data.getClientInstance() || !g_Data.getClientInstance()->getLevelRenderer()) return;

	Vec3<float> origin = g_Data.getClientInstance()->getLevelRenderer()->levelRendererPlayer->Origin;

	Vec3<float> vertices[8] = {
		{box.lower.x, box.lower.y, box.lower.z},
		{box.upper.x, box.lower.y, box.lower.z},
		{box.lower.x, box.upper.y, box.lower.z},
		{box.upper.x, box.upper.y, box.lower.z},
		{box.lower.x, box.lower.y, box.upper.z},
		{box.upper.x, box.lower.y, box.upper.z},
		{box.lower.x, box.upper.y, box.upper.z},
		{box.upper.x, box.upper.y, box.upper.z}
	};

	Vec2<float> screen[8];
	bool visible[8] = {};

	for (int i = 0; i < 8; i++)
		visible[i] = g_Data.getClientInstance()->WorldToScreen(vertices[i], screen[i]);

	const int faces[6][4] = {
		{0,1,3,2}, // Bottom
		{4,5,7,6}, // Top
		{0,1,5,4}, // Front
		{2,3,7,6}, // Back
		{1,3,7,5}, // Right
		{0,2,6,4}  // Left
	};

	ID2D1SolidColorBrush* fillBrush = nullptr;
	if (fill) {
         d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(fillColor.r / 255.f, fillColor.g / 255.f, fillColor.b / 255.f, fillColor.a / 255.f),
			&fillBrush
		);
	}

	ID2D1SolidColorBrush* lineBrush = nullptr;
	if (outline) {
		d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(outlineColor.r / 255.f, outlineColor.g / 255.f, outlineColor.b / 255.f, outlineColor.a / 255.f),
			&lineBrush
		);
	}


	if (fill) {
		for (const auto& face : faces) {
			if (visible[face[0]] && visible[face[1]] && visible[face[2]] && visible[face[3]]) {
				ID2D1PathGeometry* geometry = nullptr;
				ID2D1GeometrySink* sink = nullptr;
				ID2D1Factory* factory = nullptr;
				d2dDeviceContext->GetFactory(reinterpret_cast<ID2D1Factory**>(&factory));
				factory->CreatePathGeometry(&geometry);
				geometry->Open(&sink);

				sink->BeginFigure(D2D1::Point2F(screen[face[0]].x, screen[face[0]].y), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F tri1[] = {
					D2D1::Point2F(screen[face[1]].x, screen[face[1]].y),
					D2D1::Point2F(screen[face[2]].x, screen[face[2]].y)
				};
				sink->AddLines(tri1, 2);
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);

				sink->BeginFigure(D2D1::Point2F(screen[face[0]].x, screen[face[0]].y), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F tri2[] = {
					D2D1::Point2F(screen[face[2]].x, screen[face[2]].y),
					D2D1::Point2F(screen[face[3]].x, screen[face[3]].y)
				};
				sink->AddLines(tri2, 2);
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);

				sink->Close();
				d2dDeviceContext->FillGeometry(geometry, fillBrush);
				sink->Release();
				geometry->Release();
				factory->Release();
			}
		}
	}

	if (outline) {
		const int edges[12][2] = {
			{0,1}, {1,3}, {3,2}, {2,0},
			{4,5}, {5,7}, {7,6}, {6,4},
			{0,4}, {1,5}, {2,6}, {3,7}
		};

		for (const auto& edge : edges) {
			if (visible[edge[0]] && visible[edge[1]]) {
				Vec2<float> a = screen[edge[0]];
				Vec2<float> b = screen[edge[1]];
				d2dDeviceContext->DrawLine(D2D1::Point2F(a.x, a.y), D2D1::Point2F(b.x, b.y), lineBrush, thickness);
			}
		}
	}

	if (fillBrush) fillBrush->Release();
	if (lineBrush) lineBrush->Release();
}
void InterfaceUtils::drawBoxTest(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, const UIColor& fillColor, const UIColor& outlineColor, bool fill, bool outline, float rotationAngle, const Vec3<float>& rotationAxis) {
	std::shared_ptr<GLMatrix> refdef = std::shared_ptr<GLMatrix>(g_Data.getClientInstance()->getbadrefdef()->correct());
	Vec3<float> diff = upper.sub(lower);
	Vec3<float> origin = g_Data.getClientInstance()->getLevelRenderer()->levelRendererPlayer->Origin;
	Vec2<float> fov = g_Data.getClientInstance()->getFov();
	Vec2<float> screenSize = g_Data.getClientInstance()->guiData->windowSizeReal;

	Vec3<float> vertices[8] = {
		{lower.x, lower.y, lower.z},
		{upper.x, lower.y, lower.z},
		{lower.x, upper.y, lower.z},
		{upper.x, upper.y, lower.z},
		{lower.x, lower.y, upper.z},
		{upper.x, lower.y, upper.z},
		{lower.x, upper.y, upper.z},
		{upper.x, upper.y, upper.z}
	};

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(rotationAxis.x, rotationAxis.y, rotationAxis.z));
	Vec3<float> center = (lower.add(upper)).div(2.0f);

	for (int i = 0; i < 8; i++) {
		Vec3<float> translated = vertices[i].sub(center);
		glm::vec4 rotated = rotationMatrix * glm::vec4(translated.x, translated.y, translated.z, 1.0f);
		vertices[i] = Vec3<float>(rotated.x, rotated.y, rotated.z).add(center);
	}

	Vec2<float> screen[8];
	bool visible[8] = {};
	for (int i = 0; i < 8; i++)
		visible[i] = refdef->OWorldToScreen(origin, vertices[i], screen[i], fov, screenSize);

	const int faces[6][4] = {
		{0,1,3,2}, {4,5,7,6}, {0,1,5,4}, {2,3,7,6}, {1,3,7,5}, {0,2,6,4}
	};

	if (fill) {
		ID2D1SolidColorBrush* brush = nullptr;
		d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(fillColor.r / 255.f, fillColor.g / 255.f, fillColor.b / 255.f, fillColor.a / 255.f), &brush);
		for (auto& face : faces) {
			if (visible[face[0]] && visible[face[1]] && visible[face[2]] && visible[face[3]]) {
				ID2D1PathGeometry* geometry = nullptr;
				ID2D1GeometrySink* sink = nullptr;
				ID2D1Factory* factory = nullptr;
				d2dDeviceContext->GetFactory(reinterpret_cast<ID2D1Factory**>(&factory));
				factory->CreatePathGeometry(&geometry);
				geometry->Open(&sink);

				sink->BeginFigure(D2D1::Point2F(screen[face[0]].x, screen[face[0]].y), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F tri1[] = {
					D2D1::Point2F(screen[face[1]].x, screen[face[1]].y),
					D2D1::Point2F(screen[face[2]].x, screen[face[2]].y)
				};
				sink->AddLines(tri1, 2);
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);

				sink->BeginFigure(D2D1::Point2F(screen[face[0]].x, screen[face[0]].y), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F tri2[] = {
					D2D1::Point2F(screen[face[2]].x, screen[face[2]].y),
					D2D1::Point2F(screen[face[3]].x, screen[face[3]].y)
				};
				sink->AddLines(tri2, 2);
				sink->EndFigure(D2D1_FIGURE_END_CLOSED);

				sink->Close();
				d2dDeviceContext->FillGeometry(geometry, brush);
				sink->Release();
				geometry->Release();
				factory->Release();
			}
		}
		brush->Release();
	}

	if (outline) {
		const int edges[12][2] = {
			{0,1}, {1,3}, {3,2}, {2,0},
			{4,5}, {5,7}, {7,6}, {6,4},
			{0,4}, {1,5}, {2,6}, {3,7}
		};
		ID2D1SolidColorBrush* brush = nullptr;
		d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(outlineColor.r / 255.f, outlineColor.g / 255.f, outlineColor.b / 255.f, outlineColor.a / 255.f), &brush);
		for (const auto& edge : edges) {
			if (visible[edge[0]] && visible[edge[1]]) {
				d2dDeviceContext->DrawLine(D2D1::Point2F(screen[edge[0]].x, screen[edge[0]].y), D2D1::Point2F(screen[edge[1]].x, screen[edge[1]].y), brush, lineWidth);
			}
		}
		brush->Release();
	}
}

void InterfaceUtils::drawBoxOutline(const Vec3<float>& lower, const Vec3<float>& upper, float lineWidth, const UIColor& outlineColor, float rotationAngle, const Vec3<float>& rotationAxis) {
	std::shared_ptr<GLMatrix> refdef = std::shared_ptr<GLMatrix>(g_Data.getClientInstance()->getbadrefdef()->correct());
	Vec3<float> diff = upper.sub(lower);
	Vec3<float> origin = g_Data.getClientInstance()->getLevelRenderer()->levelRendererPlayer->Origin;
	Vec2<float> fov = g_Data.getClientInstance()->getFov();
	Vec2<float> screenSize = g_Data.getClientInstance()->guiData->windowSizeReal;

	Vec3<float> vertices[8] = {
		{lower.x, lower.y, lower.z},
		{upper.x, lower.y, lower.z},
		{lower.x, upper.y, lower.z},
		{upper.x, upper.y, lower.z},
		{lower.x, lower.y, upper.z},
		{upper.x, lower.y, upper.z},
		{lower.x, upper.y, upper.z},
		{upper.x, upper.y, upper.z}
	};

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(rotationAxis.x, rotationAxis.y, rotationAxis.z));
	Vec3<float> center = (lower.add(upper)).div(2.0f);

	for (int i = 0; i < 8; i++) {
		Vec3<float> translated = vertices[i].sub(center);
		glm::vec4 rotated = rotationMatrix * glm::vec4(translated.x, translated.y, translated.z, 1.0f);
		vertices[i] = Vec3<float>(rotated.x, rotated.y, rotated.z).add(center);
	}

	Vec2<float> screen[8];
	bool visible[8] = {};
	for (int i = 0; i < 8; i++)
		visible[i] = refdef->OWorldToScreen(origin, vertices[i], screen[i], fov, screenSize);

	const int edges[12][2] = {
		{0,1}, {1,3}, {3,2}, {2,0},
		{4,5}, {5,7}, {7,6}, {6,4},
		{0,4}, {1,5}, {2,6}, {3,7}
	};
	ID2D1SolidColorBrush* brush = nullptr;
	d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(outlineColor.r / 255.f, outlineColor.g / 255.f, outlineColor.b / 255.f, outlineColor.a / 255.f), &brush);
	for (const auto& edge : edges) {
		if (visible[edge[0]] && visible[edge[1]]) {
			d2dDeviceContext->DrawLine(D2D1::Point2F(screen[edge[0]].x, screen[edge[0]].y), D2D1::Point2F(screen[edge[1]].x, screen[edge[1]].y), brush, lineWidth);
		}
	}
	brush->Release();
}

void InterfaceUtils::fillGlowRectangle(const Vec4<float>& rect, const UIColor& color, float glowStrength) {
	ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
	D2D1_RECT_F d2dRect = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);

	// Draw the glow effect by drawing multiple rectangles with decreasing opacity
	for (int i = 0; i < 10; ++i) {
		float opacity = (10 - i) / 10.0f * glowStrength;
		UIColor glowColor = UIColor(color.r, color.g, color.b, static_cast<uint8_t>(color.a * opacity));
		ID2D1SolidColorBrush* glowBrush = getSolidColorBrush(glowColor);
		d2dDeviceContext->FillRectangle(d2dRect, glowBrush);
		d2dRect.left -= 1.0f;
		d2dRect.top -= 1.0f;
		d2dRect.right += 1.0f;
		d2dRect.bottom += 1.0f;
	}

	// Draw the main rectangle
	d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
}
void InterfaceUtils::drawColorSB(const Vec4<float>& rect, float hue) {
	int steps = 10;
	float w = rect.z - rect.x;
	float h = rect.w - rect.y;

	for (int y = 0; y < steps; ++y) {
		for (int x = 0; x < steps; ++x) {
			float s = x / (float)(steps - 1);
			float b = 1.f - y / (float)(steps - 1);

			float r, g, bVal;
			ColorUtil::ColorConvertHSVtoRGB(hue, s, b, r, g, bVal);

			UIColor rgbColor = UIColor((int)(r * 255.f), (int)(g * 255.f), (int)(bVal * 255.f));
			float sx = rect.x + x * (w / steps);
			float sy = rect.y + y * (h / steps);
			float ex = sx + (w / steps);
			float ey = sy + (h / steps);

			InterfaceUtils::FillRect(Vec4<float>(sx, sy, ex, ey), rgbColor);
		}
	}
}

void InterfaceUtils::drawColorHueBar(const Vec4<float>& rect) {
	int steps = 50;
	float stepHeight = (rect.w - rect.y) / steps;

	for (int i = 0; i < steps; ++i) {
		float hue = i / (float)(steps - 1);

		float r, g, b;
		ColorUtil::ColorConvertHSVtoRGB(hue, 1.f, 1.f, r, g, b);
		UIColor rgbColor = UIColor((int)(r * 255.f), (int)(g * 255.f), (int)(b * 255.f));

		float y = rect.y + i * stepHeight;
		InterfaceUtils::FillRect(Vec4<float>(rect.x, y, rect.z, y + stepHeight), rgbColor);
	}
}


void InterfaceUtils::drawVerticalGradient(const Vec4<float>& rect, const UIColor& startColor, const UIColor& endColor) {
	D2D1_GRADIENT_STOP gradientStops[2];
	gradientStops[0].color = startColor.toD2D1Color();
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = endColor.toD2D1Color();
	gradientStops[1].position = 1.0f;

	ID2D1GradientStopCollection* pGradientStops = nullptr;

	HRESULT hr = d2dDeviceContext->CreateGradientStopCollection(
		gradientStops,
		2,
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&pGradientStops
	);
	if (FAILED(hr) || !pGradientStops)
		return;

	ID2D1LinearGradientBrush* pLinearGradientBrush = nullptr;
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES brushProperties = {};
	brushProperties.startPoint = D2D1::Point2F(rect.x, rect.y);
	brushProperties.endPoint = D2D1::Point2F(rect.x, rect.w);

	hr = d2dDeviceContext->CreateLinearGradientBrush(brushProperties, pGradientStops, &pLinearGradientBrush);
	if (FAILED(hr) || !pLinearGradientBrush) {
		pGradientStops->Release();
		return;
	}

	d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), pLinearGradientBrush);

	pLinearGradientBrush->Release();
	pGradientStops->Release();
}

void InterfaceUtils::DrawColorWheel(Vec2<float> center, float radius) {
	const int segments = 360;
	const float angleStep = 6.2831853f / segments;

	for (int i = 0; i < segments; ++i) {
		float angle = i * angleStep;
		float nextAngle = (i + 1) * angleStep;

		float h = i / 360.f;
		Vec3<uint8_t> rgb = HSVtoRGB(h, 1.f, 1.f);
		D2D1_COLOR_F color = D2D1::ColorF(rgb.x / 255.f, rgb.y / 255.f, rgb.z / 255.f);

		Vec2<float> p1(center.x, center.y);
		Vec2<float> p2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
		Vec2<float> p3(center.x + cosf(nextAngle) * radius, center.y + sinf(nextAngle) * radius);

		ID2D1SolidColorBrush* brush = nullptr;
		d2dDeviceContext->CreateSolidColorBrush(color, &brush);

		ID2D1PathGeometry* geometry = nullptr;
		ID2D1GeometrySink* sink = nullptr;
		d2dFactory->CreatePathGeometry(&geometry);
		geometry->Open(&sink);
		sink->BeginFigure(D2D1::Point2F(p1.x, p1.y), D2D1_FIGURE_BEGIN_FILLED);
		sink->AddLine(D2D1::Point2F(p2.x, p2.y));
		sink->AddLine(D2D1::Point2F(p3.x, p3.y));
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();
		sink->Release();

		d2dDeviceContext->FillGeometry(geometry, brush);

		brush->Release();
		geometry->Release();
	}
}


Vec3<uint8_t> InterfaceUtils::HSVtoRGB(float h, float s, float v) {
	float r = 0, g = 0, b = 0;

	int i = (int)(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	switch (i % 6) {
	case 0: r = v, g = t, b = p; break;
	case 1: r = q, g = v, b = p; break;
	case 2: r = p, g = v, b = t; break;
	case 3: r = p, g = q, b = v; break;
	case 4: r = t, g = p, b = v; break;
	case 5: r = v, g = p, b = q; break;
	}

	return Vec3<uint8_t>((int)(r * 255), (int)(g * 255), (int)(b * 255));
}

Vec3<float> InterfaceUtils::GetHueFromMouse(Vec2<float> center, float radius, Vec2<float> mouse) {
	float dx = mouse.x - center.x;
	float dy = mouse.y - center.y;
	float distance = sqrtf(dx * dx + dy * dy);

	if (distance > radius)
		return Vec3<float>(-1.f, 0.f, 0.f);

	float angle = atan2f(dy, dx);
	if (angle < 0.f)
		angle += 6.2831853f;

	float hue = angle / 6.2831853f;

	return Vec3<float>(hue, 1.f, 1.f);
}

void InterfaceUtils::PushAxisAlignedClip(const Vec4<float>& rect, bool aliased) {
	if (!initD2D || !d2dDeviceContext) return;

	D2D1_RECT_F clipRect = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);
	d2dDeviceContext->PushAxisAlignedClip(
		clipRect,
		aliased ? D2D1_ANTIALIAS_MODE_ALIASED : D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
	);
}

void InterfaceUtils::PopAxisAlignedClip() {
	if (!initD2D || !d2dDeviceContext) return;
	d2dDeviceContext->PopAxisAlignedClip();
}
