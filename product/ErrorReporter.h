// Copyright © 2025 Henry Frodsham
#pragma once
#include <functional>
#include <iostream>

#include "ErrorManifest.h"
#include "EventBus.h"
#include "EventQueue.h"

class ErrorReporter {
 private:
  void ErrorOutput(Error ErrorToReport);
  EventBus ErrorBus;

  // now encapsulated since publishError exclusively interacts with the Queue
  // here
  EventQueue ErrorQueue;

 public:
  ErrorReporter();
  ~ErrorReporter() {}

  // add a handler function to a specific error, rejects base Error type
  template <typename SomeError>
  void AddHandler(std::function<void(const SomeError&)> Handler) {
    // no need to add extra handlers to base Error types, just reject them
    if constexpr (std::is_same_v<SomeError, Error>) {
      return;
    }

    ErrorBus.Subscribe<SomeError>(Handler);
  }

  template <typename SomeError>
  void EnqueueError(SomeError&& Err) {
    static_assert(std::is_base_of<Error, std::decay_t<SomeError>>::value,
                  "illegal event Queued to ErrorReporter");

    using ErrType = std::decay_t<SomeError>;

    // check if its an error derived from the base Error event
    // if so, log and handle
    // if its just an Error event then just log
    if constexpr (!std::is_same_v<ErrType, Error>) {
      // log decayed base Error event
      Error baseError(Err.Code, Err.Level, Err.Message, Err.Origin);
      ErrorQueue.Enqueue(baseError);

      // handle derived event
      // e.g DeadDeviceIdError
      ErrorQueue.Enqueue(std::forward<SomeError>(Err));
    } else {
      ErrorQueue.Enqueue(std::forward<SomeError>(Err));
    }
  }

  // Queue encapsulated so dedicated public method needed
  void Dispatch();
};
