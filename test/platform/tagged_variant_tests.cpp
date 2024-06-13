#include <gtest/gtest.h>

#include <platform/tagged_variant.h>

enum class Tag {
	A,
	B,
};

struct VariantA {
	static constexpr Tag TAG = Tag::A;
};

struct VariantB {
	static constexpr Tag TAG = Tag::B;
};

using TestVariant = platform::TaggedVariant<Tag, VariantA, VariantB>;

TEST(TaggedVariantTests, DefaultVariantHasDefaultTag) {
	TestVariant variant;
	EXPECT_EQ(variant.tag(), Tag::A);
}
