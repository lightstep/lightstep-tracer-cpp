#include <event2/event.h>
#include "qa_bot.h"

int main() {
  auto base = event_base_new();
  QABot bot{base};
  event_base_dispatch(base);
  event_base_free(base);
  return 0;
}
