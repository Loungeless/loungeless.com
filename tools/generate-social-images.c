#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <ImageIO/ImageIO.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { WIDTH = 1200, HEIGHT = 630 };

typedef struct {
    const char *filename;
    const char *title_one;
    const char *title_two;
    const char *detail;
    const char *footer;
} SocialCard;

static CGColorRef make_color(CGFloat red, CGFloat green, CGFloat blue) {
    CGFloat components[] = { red / 255.0, green / 255.0, blue / 255.0, 1.0 };
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGColorRef color = CGColorCreate(space, components);
    CGColorSpaceRelease(space);
    return color;
}

static double draw_text(
    CGContextRef context,
    const char *text,
    CGFloat x,
    CGFloat y,
    CGFloat size,
    CFStringRef font_name,
    CGColorRef color
) {
    CFStringRef string = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
    CTFontRef font = CTFontCreateWithName(font_name, size, NULL);
    const void *keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
    const void *values[] = { font, color };
    CFDictionaryRef attributes = CFDictionaryCreate(
        NULL,
        keys,
        values,
        2,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    CFAttributedStringRef attributed = CFAttributedStringCreate(NULL, string, attributes);
    CTLineRef line = CTLineCreateWithAttributedString(attributed);

    CGContextSetTextPosition(context, x, y);
    CTLineDraw(line, context);
    double width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);

    CFRelease(line);
    CFRelease(attributed);
    CFRelease(attributes);
    CFRelease(font);
    CFRelease(string);
    return width;
}

static bool write_png(CGContextRef context, const char *path) {
    CGImageRef image = CGBitmapContextCreateImage(context);
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        NULL,
        (const UInt8 *)path,
        (CFIndex)strlen(path),
        false
    );
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(
        url,
        CFSTR("public.png"),
        1,
        NULL
    );

    bool success = false;
    if (destination != NULL) {
        CGImageDestinationAddImage(destination, image, NULL);
        success = CGImageDestinationFinalize(destination);
        CFRelease(destination);
    }
    CFRelease(url);
    CGImageRelease(image);
    return success;
}

static bool render_card(const SocialCard *card, const char *output_directory) {
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
        NULL,
        WIDTH,
        HEIGHT,
        8,
        0,
        space,
        kCGImageAlphaNoneSkipLast
    );
    CGColorSpaceRelease(space);
    if (context == NULL) {
        return false;
    }

    CGColorRef background = make_color(10, 10, 11);
    CGColorRef primary = make_color(232, 232, 236);
    CGColorRef secondary = make_color(154, 154, 160);
    CGColorRef accent = make_color(200, 150, 90);

    CGContextSetFillColorWithColor(context, background);
    CGContextFillRect(context, CGRectMake(0, 0, WIDTH, HEIGHT));

    CGContextSetFillColorWithColor(context, accent);
    CGPathRef accent_bar = CGPathCreateWithRoundedRect(CGRectMake(82, 82, 8, 466), 4, 4, NULL);
    CGContextAddPath(context, accent_bar);
    CGContextFillPath(context);
    CGPathRelease(accent_bar);
    CGContextFillEllipseInRect(context, CGRectMake(1038, 503, 34, 34));

    double brand_width = draw_text(context, "LOUNGE", 126, 500, 25, CFSTR("HelveticaNeue-Medium"), primary);
    draw_text(context, "LESS", 126 + brand_width + 10, 500, 25, CFSTR("HelveticaNeue-Medium"), accent);

    draw_text(context, card->title_one, 126, 340, 57, CFSTR("HelveticaNeue-Bold"), primary);
    draw_text(context, card->title_two, 126, 266, 57, CFSTR("HelveticaNeue-Bold"), primary);
    draw_text(context, card->detail, 128, 163, 25, CFSTR("HelveticaNeue"), secondary);
    draw_text(context, card->footer, 128, 84, 21, CFSTR("HelveticaNeue-Medium"), primary);

    char path[4096];
    int length = snprintf(path, sizeof(path), "%s/%s", output_directory, card->filename);
    bool success = length > 0 && length < (int)sizeof(path) && write_png(context, path);

    CGColorRelease(accent);
    CGColorRelease(secondary);
    CGColorRelease(primary);
    CGColorRelease(background);
    CGContextRelease(context);
    return success;
}

int main(int argc, char **argv) {
    const char *output_directory = argc > 1 ? argv[1] : "assets";
    const SocialCard cards[] = {
        {
            "loungeless-social.png",
            "Lounge less,",
            "do more.",
            "AI  •  cybersecurity  •  identity  •  enterprise technology",
            "Harrison Greanias"
        },
        {
            "breach-blindspots-social.png",
            "The breach is rarely the lesson.",
            "The entry path is.",
            "Sourced enterprise security research",
            "Loungeless  •  Harrison Greanias"
        }
    };

    for (size_t index = 0; index < sizeof(cards) / sizeof(cards[0]); index++) {
        if (!render_card(&cards[index], output_directory)) {
            fprintf(stderr, "Failed to render %s\n", cards[index].filename);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
