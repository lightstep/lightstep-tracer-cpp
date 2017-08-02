#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <array>
#include <random>
#include "config.h"

static const std::string& PickQuestion() {
  static thread_local std::mt19937 rng{std::random_device{}()};
  static const std::string questions[] = {
      "Will it?\n",      "Won't I?\n",    "Wait?\n",     "Should I?\n",
      "Should he?\n",    "Should she?\n", "Does it?\n",  "Now?\n",
      "Bet the farm?\n", "Does he?\n",    "Does she?\n", "Am I right?\n"};
  std::uniform_int_distribution<> dist{
      0, sizeof(questions) / sizeof(questions[0]) - 1};
  return questions[dist(rng)];
};

void error(const char *msg) {
  perror(const_cast<char*>(msg));
  std::terminate();
}

static void WriteQuestion(int socketfd, const std::string& s) {
  int num_written = 0;
  do {
    auto n = write(socketfd, s.data() + num_written, s.size() - num_written);
    if (n <= 0) {
      std::cerr << "Error writing to socket\n";
      std::terminate();
    }
    num_written += n;
  } while (num_written < s.size());
}

static std::string ReadAnswer(int socketfd) {
  std::string result;
  std::array<char, socket_buffer_size> buffer;
  while(1) {
    auto n = read(socketfd, buffer.data(), buffer.size());
    if (n <= 0) {
      std::cerr << "Error reading from socket\n";
      std::terminate();
    }
    result.append(buffer.data(), n);
    if (result.back() == '\n') {
      return result;
    }
  }
}

static void RunQA(int socketfd) {
  const auto& question = PickQuestion();
  WriteQuestion(socketfd, question);
  auto answer = ReadAnswer(socketfd);
  std::string log;
  log.append("Ask: ");
  log.append(question);
  log.append("Answer: ");
  log.append(answer);
  std::cout << log;
}

int main(int argc, char *argv[]) {
  int socketfd, portno, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) error("ERROR opening socket");
  server = gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(qabot_port);
  if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  RunQA(socketfd);
  return 0;
}
