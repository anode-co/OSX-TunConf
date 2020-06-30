#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

static CFMutableDictionaryRef dictSetKey(CFMutableDictionaryRef dict, char* key_c, const void* val)
{
    CFStringRef key = CFStringCreateWithCString(NULL, key_c, kCFStringEncodingUTF8);
    CFMutableDictionaryRef dict1 = CFDictionaryCreateMutableCopy(NULL, 0, dict);
    CFDictionarySetValue(dict1, key, val);
	CFRelease(key);
	CFRelease(dict);
    CFRelease(val); // consumes val
    return dict1;
}

static CFMutableArrayRef mkArray(char** values)
{
	CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    for (int i = 0; values[i]; i++) {
        CFTypeRef val = (CFPropertyListRef) CFStringCreateWithCString(
                NULL, values[i], kCFStringEncodingUTF8);
        CFArrayAppendValue(array, val);
        CFRelease(val);
    }
	return array;
}

static CFPropertyListRef mkString(char* val)
{
    return (CFPropertyListRef) CFStringCreateWithCString(NULL, val, kCFStringEncodingUTF8);
}

static void reconnected(SCDynamicStoreRef store, void *info)
{
	printf("SCDynamicStore server restarted, session reconnected\n");
}

static void storeCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
	printf("notification callback (store address = %p), %d keys changed\n",
        store, (int)(long) CFArrayGetCount(changedKeys));
	return;
}

static SCDynamicStoreRef openStore()
{
    return SCDynamicStoreCreate(NULL, CFSTR("scutil"), storeCallback, NULL);
}

static void set(SCDynamicStoreRef store, char* key_c, CFPropertyListRef value)
{
	CFStringRef	key = CFStringCreateWithCString(NULL, key_c, kCFStringEncodingUTF8);
	if (!SCDynamicStoreSetValue(store, key, value)) {
        printf("Failed to set [%s]: [%s]\n", key_c, SCErrorString(SCError()));
	}
	CFRelease(key);
	return;
}

static void remove(SCDynamicStoreRef store, char* key_c)
{
	CFStringRef key = CFStringCreateWithCString(NULL, key_c, kCFStringEncodingUTF8);
	if (!SCDynamicStoreRemoveValue(store, key)) {
		printf("Failed to remove [%s]: [%s]\n", key_c, SCErrorString(SCError()));
	}
	CFRelease(key);
	return;
}

static CFMutableDictionaryRef mkDict()
{
    return CFDictionaryCreateMutable(
        NULL,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
}

/*
d.init
d.add Addresses * 10.66.6.0
d.add DestAddresses * 28.30.0.0
d.add InterfaceName utun2
set State:/Network/Service/wg-updown-utun2/IPv4
set Setup:/Network/Service/wg-updown-utun2/IPv4
d.init
d.add Addresses * fe80::a65e:60ff:feea:ab95 fca4:aa4c:3686:6a29:e301:89a5:942c:38d3 2c0f:f930:2::
d.add DestAddresses * ::ffff:ffff:ffff:ffff:0:0 :: ::
d.add Flags * 0 0 0
d.add InterfaceName utun2
d.add PrefixLength * 64 8 64
set State:/Network/Service/wg-updown-utun2/IPv6
set Setup:/Network/Service/wg-updown-utun2/IPv6
*/

int main()
{
    SCDynamicStoreRef store = openStore();

    CFMutableDictionaryRef d = mkDict();
    d = dictSetKey(d, "Addresses", mkArray((char*[]){ "10.66.6.0", NULL }));
    d = dictSetKey(d, "DestAddresses", mkArray((char*[]){ "28.30.0.0", NULL }));
    d = dictSetKey(d, "InterfaceName", mkString("utun2"));
    set(store, "State:/Network/Service/wg-updown-utun2/IPv4", d);
    set(store, "Setup:/Network/Service/wg-updown-utun2/IPv4", d);
    CFRelease(d);

    d = mkDict();
    d = dictSetKey(d, "Addresses", mkArray((char*[]){
        "fe80::a65e:60ff:feea:ab95",
        "fca4:aa4c:3686:6a29:e301:89a5:942c:38d3",
        "2c0f:f930:2::",
        NULL,
    }));
    d = dictSetKey(d, "DestAddresses", mkArray((char*[]){
        "::ffff:ffff:ffff:ffff:0:0",
        "::",
        "::",
        NULL,
    }));
    d = dictSetKey(d, "Flags", mkArray((char*[]){
        "0",
        "0",
        "0",
        NULL,
    }));
    d = dictSetKey(d, "PrefixLength", mkArray((char*[]){
        "64",
        "8",
        "64",
        NULL,
    }));
    d = dictSetKey(d, "InterfaceName", mkString("utun2"));
    set(store, "State:/Network/Service/wg-updown-utun2/IPv6", d);
    set(store, "Setup:/Network/Service/wg-updown-utun2/IPv6", d);
    CFRelease(d);

    // ifdown -> remove(store, "State:/Network/Service/wg-updown-utun2/IPv6")
}