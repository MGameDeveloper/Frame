
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

enum masEKey
{
	EKey_Unknown,

	EKey_W = 1,
	EKey_S = 2,
	EKey_ArrowUp = 3,
	EKey_ArrowDown = 4,
	EKey_LAnalogUp,
	EKey_LAnalogDown,
	EKey_Square,
	EKey_L1,

	EKey_Count
};
const char* EKeyNames[EKey_Count] =
{
"EKey_Unkown",
"EKey_W",
"EKey_S",
"EKey_ArrowUp",
"EKey_ArrowDown",
"EKey_LAnalogUp",
"EKey_LAnalogDown",
"EKey_Square",
"EKey_L1",
};

enum masEKeyModifier_
{
	EKeyMod_None,
	EKeyMod_Shift,
};

typedef uint32_t masEKeyModifier;

struct masKeyCpp
{
	uint8_t Keys[7];
	uint8_t KeyCount : 3;
	uint8_t bAnyKey : 1;

	masKeyCpp(const masEKey& Key)
	{
		assert((Key > EKey_Unknown && Key < EKey_Count) && "Constructing masKeyCpp must be with valid key value");

		::memset(this, 0, sizeof(*this));
		Keys[KeyCount++] = Key;
	}

	void IsAnyKeyActive(bool v)
	{
		bAnyKey = v;
	}

	masKeyCpp& operator|(const masEKey& Key)
	{
		assert(bAnyKey != 0 && "Keys must be combined by | or & but not mixed in same keys");
		assert(KeyCount < 7 && "Keys combination must not exceed 7 keys");
		Keys[KeyCount++] = Key;

		return *this;
	}

	masKeyCpp& operator&(const masEKey& Key)
	{
		assert(bAnyKey != 1 && "Keys must be combined by | or & but not mixed in same keys");
		assert(KeyCount < 7 && "Keys combination must not exceed 7 keys");
		Keys[KeyCount++] = Key;

		return *this;
	}
};

masKeyCpp operator|(const masEKey& LKey, const masEKey& RKey)
{
	masKeyCpp Out(LKey);
	Out.IsAnyKeyActive(true);
	Out = Out | RKey;
	return Out;
}

masKeyCpp operator&(const masEKey& LKey, const masEKey& RKey)
{
	masKeyCpp Out(LKey);
	Out.IsAnyKeyActive(false);
	Out = Out & RKey;
	return Out;
}


bool masInputCpp_OnAxisKey(masEKeyModifier KeyMods, const masKeyCpp& Key)
{
	printf("bAnyKey : %s\n", (Key.bAnyKey) ? "True" : "False");
	printf("KeyCount: %d\n", Key.KeyCount);
	for (int8_t i = 0; i < Key.KeyCount; ++i)
		printf("Key[%d] : %s\n", i, EKeyNames[Key.Keys[i]]);
	printf("\n");

	return false;
}

int main(int argc, const char** argv)
{
	if (masInputCpp_OnAxisKey(EKeyMod_Shift, EKey_W | EKey_ArrowUp | EKey_L1 | EKey_Square))
		printf("RUNNING_FORWARD[ %.2f ]\n", 1.f);
	else if (masInputCpp_OnAxisKey(EKeyMod_Shift, EKey_S & EKey_ArrowDown | EKey_LAnalogUp))
		printf("RUNNING_FORWARD[ %.2f ]\n", -1.f);

	return 0;
}