// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_PLATFORM_UPDATE_ENGINE_SUBPROCESS_H__
#define CHROMEOS_PLATFORM_UPDATE_ENGINE_SUBPROCESS_H__

#include <map>
#include <string>
#include <vector>
#include <glib.h>
#include "base/basictypes.h"
#include "base/logging.h"

// The Subprocess class is a singleton. It's used to spawn off a subprocess
// and get notified when the subprocess exits. The result of Exec() can
// be saved and used to cancel the callback request. If you know you won't
// call CancelExec(), you may safely lose the return value from Exec().

namespace chromeos_update_engine {

class Subprocess {
 public:
  static void Init() {
    CHECK(!subprocess_singleton_);
    subprocess_singleton_ = new Subprocess;
  }
   
  typedef void(*ExecCallback)(int return_code, void *p);

  // Returns a tag > 0 on success.
  uint32_t Exec(const std::vector<std::string>& cmd,
                ExecCallback callback,
                void* p);

  // Used to cancel the callback. The process will still run to completion.
  void CancelExec(uint32_t tag);

  // Executes a command synchronously. Returns true on success.
  static bool SynchronousExecFlags(const std::vector<std::string>& cmd,
                                   int* return_code,
                                   GSpawnFlags flags);
  static bool SynchronousExec(const std::vector<std::string>& cmd,
                              int* return_code) {
    return SynchronousExecFlags(cmd, return_code, static_cast<GSpawnFlags>(0));
  }

  // Gets the one instance
  static Subprocess& Get() {
    return *subprocess_singleton_;
  }
  
  // Returns true iff there is at least one subprocess we're waiting on.
  bool SubprocessInFlight() {
    for (std::map<int, SubprocessCallbackRecord>::iterator it =
             callback_records_.begin();
         it != callback_records_.end(); ++it) {
      if (it->second.callback)
        return true;
    }
    return false;
  }
 private:
  // The global instance
  static Subprocess* subprocess_singleton_;

  // Callback for when any subprocess terminates. This calls the user
  // requested callback.
  static void GChildExitedCallback(GPid pid, gint status, gpointer data);

  // Callback which runs in the child before exec to redirect stderr onto
  // stdout.
  static void GRedirectStderrToStdout(gpointer user_data);

  struct SubprocessCallbackRecord {
    ExecCallback callback;
    void* callback_data;
  };

  std::map<int, SubprocessCallbackRecord> callback_records_;

  Subprocess() {}
  DISALLOW_COPY_AND_ASSIGN(Subprocess);
};

}  // namespace chromeos_update_engine

#endif  // CHROMEOS_PLATFORM_UPDATE_ENGINE_SUBPROCESS_H__
