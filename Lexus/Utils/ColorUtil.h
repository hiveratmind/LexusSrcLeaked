#pragma once
#include <string>
#include <stdint.h>
#include <d2d1.h>

#include "../SDK/Core/mce.h"

struct UIColor {
	union
	{
		struct {
			uint8_t r, g, b, a;
		};
		int arr[4];
	};

	UIColor(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255) {
		this->r = red;
		this->g = green;
		this->b = blue;
		this->a = alpha;
	}

	UIColor(const mce::Color& color) {
		this->r = (int)(color.r * 255.f);
		this->g = (int)(color.g * 255.f);
		this->b = (int)(color.b * 255.f);
		this->a = (int)(color.a * 255.f);
	}

	bool operator==(const UIColor& other) const {
		return (r == other.r && g == other.g && b == other.b && a == other.a);
	}

	D2D1_COLOR_F toD2D1Color() const {
		return D2D1_COLOR_F((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f);
	}

	mce::Color toMCColor() const {
		return mce::Color((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f);
	}

    // Add to UIColor class
    struct HSV {
        float h; // 0-360
        float s; // 0-1
        float v; // 0-1
    };

    HSV toHSV() const {
        HSV hsv;
        float r = r / 255.f;
        float g = g / 255.f;
        float b = b / 255.f;

        float max = std::max(r, std::max(g, b));
        float min = std::min(r, std::min(g, b));
        float delta = max - min;

        hsv.v = max;

        if (delta < 0.00001f) {
            hsv.h = 0;
            hsv.s = 0;
            return hsv;
        }

        hsv.s = delta / max;

        if (r >= max) {
            hsv.h = (g - b) / delta;
        }
        else if (g >= max) {
            hsv.h = 2.0f + (b - r) / delta;
        }
        else {
            hsv.h = 4.0f + (r - g) / delta;
        }

        hsv.h *= 60.0f;
        if (hsv.h < 0.0f) {
            hsv.h += 360.0f;
        }

        return hsv;
    }

    static UIColor fromHSV(const HSV& hsv) {
        float h = hsv.h;
        float s = hsv.s;
        float v = hsv.v;

        if (s <= 0.0f) {
            return UIColor(
                (uint8_t)(v * 255),
                (uint8_t)(v * 255),
                (uint8_t)(v * 255)
            );
        }

        float hh = h;
        if (hh >= 360.0f) hh = 0.0f;
        hh /= 60.0f;
        int i = (int)hh;
        float ff = hh - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - (s * ff));
        float t = v * (1.0f - (s * (1.0f - ff)));

        float r, g, b;
        switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5:
        default: r = v; g = p; b = q; break;
        }

        return UIColor(
            (uint8_t)(r * 255),
            (uint8_t)(g * 255),
            (uint8_t)(b * 255)
        );
    }
};

namespace ColorUtil {
	uint32_t ColorToUInt(const UIColor& color);
	std::string ColorToHexString(const UIColor& color);
	UIColor HexStringToColor(std::string hexString);
	void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);
	UIColor lerp(const UIColor& start, const UIColor& end, float t);
	UIColor getRainbowColor(float seconds, float saturation, float brightness, long index);
	UIColor getWaveColor(const UIColor& startColor, const UIColor& endColor, long index);
    UIColor astolfoRainbow(int yOffset, int yTotal, int alpha = 255);
}
