// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: CC0-1.0

#include <iostream>
#include <ranges>
#include <QXmppClient.h>
#include <QCoreApplication>
#include <QFuture>
#include <QDomElement>
#include <QFutureWatcher>
#include <QXmppMixIq.h>
#include <QXmppDiscoveryIq.h>
#include <QXmppDiscoveryManager.h>

namespace ranges = std::ranges;

using namespace std;
using namespace QXmpp;

template<typename T, typename Handler>
void await(const QFuture<T> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<T>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler = std::move(handler)]() mutable {
                         handler(watcher->result());
                         watcher->deleteLater();
                     });
    watcher->setFuture(future);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    
    auto client = new QXmppClient(&app);
    client->logger()->setLoggingType(QXmppLogger::StdoutLogging);

    QXmppConfiguration config;
    config.setJid("lnj@deimos");
    config.setHost("localhost");
    config.setPassword("12345");
    config.setPort(5222);
    config.setIgnoreSslErrors(true);
    client->connectToServer(config);

    QObject::connect(client, &QXmppClient::connected, [client] {
        QXmppMixIq iq;
        iq.setActionType(QXmppMixIq::Create);
        iq.setChannelName("kekse2");
        iq.setTo("mix.deimos");
        iq.setType(QXmppIq::Set);

//        await(client->sendGenericIq(std::move(iq)), client, [](QXmppClient::EmptyResult result) {
//            if (const auto err = std::get_if<QXmppStanza::Error>(&result)) {
//                qDebug() << "ERROR!!!!!!!" << err->text();
//                return;
//            }
//            qDebug() << "Created!!";
//        });
        
        QXmppMixIq join;
        join.setActionType(QXmppMixIq::ClientJoin);
        join.setJid("kekse2@mix.deimos");
        join.setTo("lnj@deimos");
        join.setType(QXmppIq::Set);
        join.setNodes({ "urn:xmpp:mix:messages" });
        client->sendGenericIq(std::move(join));

        auto discoManager = client->findExtension<QXmppDiscoveryManager>();
//        await(discoManager->requestDiscoInfo("mix.deimos"), client, [](auto) {});
    });

    return app.exec();
}
