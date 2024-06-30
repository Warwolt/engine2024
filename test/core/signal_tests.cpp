#include <gtest/gtest.h>

#include <core/signal.h>

#include <string>

TEST(SignalTests, BooleanSignal_ImplicityConvertsToBool) {
	core::Signal<bool> signal = true;
	EXPECT_EQ(signal, true);
}

TEST(SignalTests, IntSignal_ImplicityConvertsToInt) {
	core::Signal<int> signal = 123;
	EXPECT_EQ(signal, 123);
}

TEST(SignalTests, IntSignal_WhenAssigned_TakesThatValue) {
	core::Signal<int> signal = 123;
	signal = 234;
	EXPECT_EQ(signal, 234);
}

TEST(SignalTests, IntSignal_WhenAssigned_JustBecameThatValue) {
	core::Signal<int> signal = 123;
	signal = 234;
	EXPECT_TRUE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenAssignedTwice_DidNotJustBecomeThatValue) {
	core::Signal<int> signal = 123;
	signal = 234;
	signal = 234;
	EXPECT_FALSE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenCopyAssigned_BecomesThatValue) {
	core::Signal<int> signal = 123;
	core::Signal<int> signal2 = 234;
	signal = signal2;
	EXPECT_TRUE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenCopyAssignedTwice_DidNotJustBecomeThatValue) {
	core::Signal<int> signal = 123;
	core::Signal<int> signal2 = 234;
	signal = signal2;
	signal = signal2;
	EXPECT_FALSE(signal.just_became(234));
}

TEST(SignalTests, StringSignal_CanBeAssignedConstChar) {
	core::Signal<std::string> signal;
	const char* string = "Hello World";
	signal = "Hello World";
	EXPECT_STREQ(signal.value().c_str(), string);
}
