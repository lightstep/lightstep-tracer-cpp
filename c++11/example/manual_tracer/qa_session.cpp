#include "qa_session.h"
#include <errno.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <random>
#include "config.h"

//------------------------------------------------------------------------------
// AnswerQuestion
//------------------------------------------------------------------------------
static std::string AnswerQuestion(const std::string& /*question*/) {
  static thread_local std::mt19937 rng{std::random_device{}()};
  static const char* const answers[] = {"It is certain\n",
                                        "It is decidedly so\n",
                                        "Without a doubt\n",
                                        "Yes definitely\n",
                                        "You may rely on it\n",
                                        "As I see it, yes\n",
                                        "Most likely\n",
                                        "Outlook good\n",
                                        "Yes\n",
                                        "Signs point to yes\n",
                                        "Reply hazy try again\n",
                                        "Ask again later\n",
                                        "Better not tell you now\n",
                                        "Cannot predict now\n",
                                        "Concentrate and ask again\n",
                                        "Don't count on it\n",
                                        "My reply is no\n",
                                        "My sources say no\n",
                                        "Outlook not so good\n",
                                        "Very doubtful\n"};
  std::uniform_int_distribution<> dist{
      0, sizeof(answers) / sizeof(answers[0]) - 1};
  return answers[dist(rng)];
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
QASession::QASession(std::unique_ptr<opentracing::Span>&& span,
                     evutil_socket_t socketfd, event_base* base)
    : span_{std::move(span)} {
  event_ = event_new(base, socketfd, EV_READ | EV_WRITE | EV_PERSIST,
                     &QASession::EventCallback, this);
  if (event_ == nullptr) {
    std::cerr << "Failed to allocate QASession event\n";
    std::terminate();
  }

  if (event_add(event_, nullptr) != 0) {
    std::cerr << "Failed to add QASession event\n";
    std::terminate();
  }
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
QASession::~QASession() { event_free(event_); }

//------------------------------------------------------------------------------
// EventCallback
//------------------------------------------------------------------------------
void QASession::EventCallback(evutil_socket_t socketfd, short what,
                              void* context) {
  auto session = static_cast<QASession*>(context);
  if (what & EV_READ) {
    session->ReadQuestions(socketfd);
  } else {
    session->WriteAnswers(socketfd);
  }
}

//------------------------------------------------------------------------------
// ReadQuestions
//------------------------------------------------------------------------------
void QASession::ReadQuestions(evutil_socket_t socketfd) {
  std::array<char, socket_buffer_size> buffer;
  int num_read;
  do {
    num_read = read(socketfd, buffer.data(), static_cast<int>(buffer.size()));

    // Reached EOF, close the socket.
    if (num_read == 0) {
      close(socketfd);
      delete this;
      return;
    }

    // Check for errors.
    if (num_read < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      std::cerr << "Error reading QASession socket\n";
      std::terminate();
    }

    auto first = buffer.data();
    auto last = buffer.data() + num_read;
    auto i = std::find(first, last, '\n');
    active_question_.append(first, i);
    if (i != last) {
      auto answer = AnswerQuestion(active_question_);
      span_->Log({{"event", "question_answer"},
                  {"question", active_question_},
                  {"answer", answer}});
      answers_.append(std::move(answer));
      active_question_.clear();
      active_question_.append(std::next(i), last);
    }
  } while (num_read == buffer.size());
}

//------------------------------------------------------------------------------
// WriteAnswers
//------------------------------------------------------------------------------
void QASession::WriteAnswers(evutil_socket_t socketfd) {
  if (write_index_ == answers_.size()) {
    return;
  }

  auto num_written = write(socketfd, answers_.data() + write_index_,
                           answers_.size() - write_index_);

  // Check for errors.
  if (num_written < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    std::cerr << "Error reading QASession socket\n";
    std::terminate();
  }

  write_index_ += num_written;
}
