#include <gtest/gtest.h>

#include <core/tagged_variant.h>

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

using TestVariant = core::TaggedVariant<Tag, VariantA, VariantB>;

TEST(TaggedVariantTests, DefaultVariantHasDefaultTag) {
	TestVariant variant;
	EXPECT_EQ(variant.tag(), Tag::A);
}

TEST(TaggedVariantTests, DefaultVariantHasDefaultValue) {
	TestVariant variant;
	EXPECT_TRUE(std::holds_alternative<VariantA>(variant));
}

TEST(TaggedVariantTests, ConstructorAssignsTag) {
	TestVariant variant = VariantB();
	EXPECT_EQ(variant.tag(), Tag::B);
}

TEST(TaggedVariantTests, ConstructorAssignsValue) {
	TestVariant variant = VariantB();
	EXPECT_TRUE(std::holds_alternative<VariantB>(variant));
}

TEST(TaggedVariantTests, AssignmentOperatorAssignsTag) {
	TestVariant variant = VariantA();
	variant = VariantB();
	EXPECT_EQ(variant.tag(), Tag::B);
}
