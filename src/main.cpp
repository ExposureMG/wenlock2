#include <dpp/dpp.h>
#include <dotenv.h>
 
int main() {q
    dotenv::init();
    dpp::cluster bot(dotenv::getenv("TOKEN"));

    bot.on_log(&dpp::utility::logcallback);

    bot.on_message_create([&bot](const dpp::message_create_t &event){});

    bot.start(dpp::st_return);
}