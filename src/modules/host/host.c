#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/host/host.h"
#include "modules/host/host.h"
#include "util/stringUtils.h"

#define FF_HOST_NUM_FORMAT_ARGS 7

void ffPrintHost(FFHostOptions* options)
{
    FFHostResult host;
    ffStrbufInit(&host.family);
    ffStrbufInit(&host.name);
    ffStrbufInit(&host.version);
    ffStrbufInit(&host.sku);
    ffStrbufInit(&host.serial);
    ffStrbufInit(&host.uuid);
    ffStrbufInit(&host.vendor);

    const char* error = ffDetectHost(&host, options);
    if(error)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(host.family.length == 0 && host.name.length == 0)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();

        if(host.name.length > 0)
            ffStrbufAppend(&output, &host.name);
        else
            ffStrbufAppend(&output, &host.family);

        if(host.version.length > 0)
            ffStrbufAppendF(&output, " (%s)", host.version.chars);

        ffStrbufPutTo(&output, stdout);
    }
    else
    {
        ffPrintFormat(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.family},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.version},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.sku},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.serial},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.uuid},
        });
    }

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);
}

bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_HOST_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    #ifdef _WIN32
    if (ffStrEqualsIgnCase(subKey, "use-wmi"))
    {
        options->useWmi = ffOptionParseBoolean(value);
        return true;
    }
    #endif

    return false;
}

void ffParseHostJsonObject(FFHostOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        #ifdef _WIN32
        if (ffStrEqualsIgnCase(key, "useWmi"))
        {
            options->useWmi = yyjson_get_bool(val);
            continue;
        }
        #endif

        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateHostJsonConfig(FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyHostOptions))) FFHostOptions defaultOptions;
    ffInitHostOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    #ifdef _WIN32
    if (options->useWmi != defaultOptions.useWmi)
        yyjson_mut_obj_add_bool(doc, module, "useWmi", options->useWmi);
    #endif
}

void ffGenerateHostJsonResult(FF_MAYBE_UNUSED FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFHostResult host;
    ffStrbufInit(&host.family);
    ffStrbufInit(&host.name);
    ffStrbufInit(&host.version);
    ffStrbufInit(&host.sku);
    ffStrbufInit(&host.serial);
    ffStrbufInit(&host.uuid);
    ffStrbufInit(&host.vendor);

    const char* error = ffDetectHost(&host, options);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (host.family.length == 0 && host.name.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "family", &host.family);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &host.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &host.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "sku", &host.sku);
    yyjson_mut_obj_add_strbuf(doc, obj, "vender", &host.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "serial", &host.serial);
    yyjson_mut_obj_add_strbuf(doc, obj, "uuid", &host.uuid);

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);
}

void ffPrintHostHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_HOST_MODULE_NAME, "{2} {3}", FF_HOST_NUM_FORMAT_ARGS, (const char* []) {
        "product family",
        "product name",
        "product version",
        "product sku",
        "product vendor",
        "product serial number",
        "product uuid",
    });
}

void ffInitHostOptions(FFHostOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_HOST_MODULE_NAME,
        "Print product name of your computer",
        ffParseHostCommandOptions,
        ffParseHostJsonObject,
        ffPrintHost,
        ffGenerateHostJsonResult,
        ffPrintHostHelpFormat,
        ffGenerateHostJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    #ifdef _WIN32
    options->useWmi = false;
    #endif
}

void ffDestroyHostOptions(FFHostOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
