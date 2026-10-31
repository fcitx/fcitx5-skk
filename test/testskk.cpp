/*
 * SPDX-FileCopyrightText: 2026~2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/testing.h>
#include <fcitx/action.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/userinterfacemanager.h>
#include "testdir.h"
#include "testfrontend_public.h"

using namespace fcitx;

void testShareStateAll(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        // Simulate Behavior/ShareInputState=All. The test environment cannot
        // write the global config file that Instance::reloadConfig() reads,
        // so apply the resulting propagate policy directly.
        instance->inputContextManager().setPropertyPropagatePolicy(
            PropertyPropagatePolicy::All);

        auto *skk = instance->addonManager().addon("skk", true);
        FCITX_ASSERT(skk);

        auto defaultGroup = instance->inputMethodManager().currentGroup();
        defaultGroup.inputMethodList().clear();
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("keyboard-us"));
        defaultGroup.inputMethodList().push_back(InputMethodGroupItem("skk"));
        defaultGroup.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(defaultGroup);

        auto *testfrontend = instance->addonManager().addon("testfrontend");
        FCITX_ASSERT(testfrontend);
        auto *latinAction = instance->userInterfaceManager().lookupAction(
            "skk-input-mode-latin");
        FCITX_ASSERT(latinAction);
        auto *katakanaAction = instance->userInterfaceManager().lookupAction(
            "skk-input-mode-katakana");
        FCITX_ASSERT(katakanaAction);

        auto uuid1 =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic1 = instance->inputContextManager().findByUUID(uuid1);
        FCITX_ASSERT(ic1);
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid1, Key("Control+space"), false));
        FCITX_ASSERT(instance->inputMethod(ic1) == "skk");
        FCITX_ASSERT(ic1->hasFocus());

        latinAction->activate(ic1);
        FCITX_ASSERT(latinAction->isChecked(ic1));
        ic1->updateProperty("skkState");

        // The dummy input context holds the shared state.
        auto *dummy = instance->inputContextManager().dummyInputContext();
        FCITX_ASSERT(dummy);
        FCITX_ASSERT(latinAction->isChecked(dummy));

        // Creating an IC copies the dummy state (including the input mode)
        // during the initial property registration. The new IC does not have
        // the focus yet, so updateInputMode() must not show the input method
        // information while copyTo() is running.
        auto uuid2 =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic2 = instance->inputContextManager().findByUUID(uuid2);
        FCITX_ASSERT(ic2);
        FCITX_ASSERT(ic2 != ic1);
        FCITX_ASSERT(!ic2->hasFocus());
        FCITX_ASSERT(latinAction->isChecked(ic2));
        FCITX_ASSERT(ic2->inputPanel().overlayMessage().empty());

        // A focused IC should still show the information when the mode
        // changes.
        ic2->focusIn();
        FCITX_ASSERT(ic2->hasFocus());
        ic2->inputPanel().reset();
        katakanaAction->activate(ic2);
        FCITX_ASSERT(katakanaAction->isChecked(ic2));
        FCITX_ASSERT(!ic2->inputPanel().overlayMessage().empty());

        instance->exit();
    });
}

int main() {
    setupTestingEnvironment(TESTING_BINARY_DIR, {"bin"},
                            {TESTING_BINARY_DIR "/test"});
    char arg0[] = "testskk";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,skk";
    char *argv[] = {arg0, arg1, arg2};
    fcitx::Log::setLogRule("default=5,skk=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    testShareStateAll(&instance);
    instance.exec();

    return 0;
}
