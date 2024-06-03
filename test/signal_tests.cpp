#include <gtest/gtest.h>

#include <platform/signal.h>

TEST(SignalTests, BooleanSignal_ImplicityConvertsToBool) {
	platform::Signal<bool> signal = true;
	EXPECT_EQ(signal, true);
}

TEST(SignalTests, IntSignal_ImplicityConvertsToInt) {
	platform::Signal<int> signal = 123;
	EXPECT_EQ(signal, 123);
}

TEST(SignalTests, IntSignal_WhenAssigned_TakesThatValue) {
	platform::Signal<int> signal = 123;
	signal = 234;
	EXPECT_EQ(signal, 234);
}

TEST(SignalTests, IntSignal_WhenAssigned_JustBecameThatValue) {
	platform::Signal<int> signal = 123;
	signal = 234;
	EXPECT_TRUE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenAssignedTwice_DidNotJustBecomeThatValue) {
	platform::Signal<int> signal = 123;
	signal = 234;
	signal = 234;
	EXPECT_FALSE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenCopyAssigned_BecomesThatValue) {
	platform::Signal<int> signal = 123;
	platform::Signal<int> signal2 = 234;
	signal = signal2;
	EXPECT_TRUE(signal.just_became(234));
}

TEST(SignalTests, IntSignal_WhenCopyAssignedTwice_DidNotJustBecomeThatValue) {
	platform::Signal<int> signal = 123;
	platform::Signal<int> signal2 = 234;
	signal = signal2;
	signal = signal2;
	EXPECT_FALSE(signal.just_became(234));
}
