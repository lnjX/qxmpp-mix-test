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
#include <QXmppRosterManager.h>
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

    enum Action { None, Join, Leave, Create, Destroy, Subscribe } action = None;
    auto actionString = std::string_view(argv[1]);
    if (actionString == "join") {
        action = Join;
    } else if (actionString == "leave") {
        action = Leave;
    } else if (actionString == "create") {
        action = Create;
    } else if (actionString == "destroy") {
        action = Destroy;
    } else if (actionString == "subscribe") {
        action = Subscribe;
    }

    QXmppConfiguration config;
    config.setJid("lnj@deimos");
    config.setHost("localhost");
    config.setPassword("12345");
    config.setResource(QString::number(QDateTime::currentDateTimeUtc().currentSecsSinceEpoch(), 36));
    config.setPort(5222);
    config.setIgnoreSslErrors(true);
    client->connectToServer(config);

    QObject::connect(client, &QXmppClient::connected, [client, action] {
        qDebug() << "Connected!";
        qDebug() << action;
        client->logger()->setLoggingType(QXmppLogger::StdoutLogging);

        QXmppMixIq iq;
        iq.setType(QXmppIq::Set);
        iq.setId("mix-req-1");
        auto channelName = QStringLiteral("kekse2");
	auto mixService = QStringLiteral("mix.voyager");
        switch (action) {
        case Join:
            iq.setActionType(QXmppMixIq::ClientJoin);
            iq.setJid(channelName + "@" + mixService);
            iq.setTo("lnj@deimos");
            iq.setNodes({ "urn:xmpp:mix:nodes:messages" });
            break;
        case Leave:
            iq.setActionType(QXmppMixIq::ClientLeave);
            iq.setJid(channelName + "@" + mixService);
            iq.setTo("lnj@deimos");
            break;
        case Create:
            iq.setActionType(QXmppMixIq::Create);
            iq.setChannelName(channelName);
            iq.setTo(mixService);
            break;
        case Destroy:
            iq.setActionType(QXmppMixIq::Destroy);
            iq.setChannelName(channelName);
            iq.setTo(mixService);
            break;
        case Subscribe:
            iq.setActionType(QXmppMixIq::UpdateSubscription);
            iq.setTo(channelName + "@" + mixService);
            iq.setNodes({"urn:xmpp:mix:messages"});
            break;
        }
        auto future = client->sendGenericIq(std::move(iq));

        QObject::connect(&client->rosterManager(), &QXmppRosterManager::itemAdded,
                         [](const QString &bareJid) {
            qDebug() << '\n' << "Item Added!" << bareJid << '\n';
        });
        QObject::connect(&client->rosterManager(), &QXmppRosterManager::itemRemoved,
                         [](const QString &bareJid) {
            qDebug() << '\n' << "Item Removed!" << bareJid << '\n';
        });
        await(future, client, [client](QXmppClient::EmptyResult result) {
            if (const auto err = std::get_if<QXmppStanza::Error>(&result)) {
                qDebug() << "\nError:" << err->text() << '\n';
                return;
            }
            qDebug() << "\nSuccess!\n";
//             client->disconnectFromServer();
        });

//         auto discoManager = client->findExtension<QXmppDiscoveryManager>();
//        await(discoManager->requestDiscoInfo("mix.deimos"), client, [](auto) {});
    });

    return app.exec();
}
