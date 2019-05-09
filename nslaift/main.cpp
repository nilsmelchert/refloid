#include <iostream>
#include <QCoreApplication>
#include "spdlog/spdlog.h"

#include "src/host/RT_object.h"
#include "src/host/RT_scene.h"
#include "src/host/RT_camera.h"

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <optixu_math_namespace.h>

#include <zmq.hpp>
#include <string>

const char *const SAMPLE_NAME = "nslaift";

void parse_data(RT_scene* scene, QString& zmq_rec_data);

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Starting raytracing application");
    auto Scene = new RT_scene();

    Scene->setBackgroundColor(0.7f, 0.7f, 0.7f);

    //  Prepare our context and socket
    zmq::context_t zmq_context(1);
    zmq::socket_t socket(zmq_context, ZMQ_REP);
    socket.bind("tcp://*:5555");
    QString zmq_request;

    while (std::cin.good()) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv(&request);
        zmq_request = QString::fromStdString(std::string(static_cast<char*>(request.data()), request.size()));
        spdlog::debug("Received String via ZMQ: \"{}\"", zmq_request.toUtf8().constData());

        parse_data(Scene, zmq_request);

        zmq::message_t reply(1);
        memcpy((void *) reply.data(), "0", 1);
        socket.send(reply);
    }

    return 0;
}

void parse_data(RT_scene* scene, QString& zmq_rec_data)
{
    QStringList sList = zmq_rec_data.split(";");
    if (0 == sList.at(0).compare("createObject", Qt::CaseInsensitive)){
        scene->createObject(sList.at(1), sList.at(2));
    } else if (0 == sList.at(0).compare("manipulateObject", Qt::CaseInsensitive)){
        scene->manipulateObject(sList.at(1), sList.at(2), sList.at(3));
    } else if (0 == sList.at(0).compare("render", Qt::CaseInsensitive)) {
        scene->updateCaches();
        scene->render();
    } else if (0 == sList.at(0).compare("deleteObject", Qt::CaseInsensitive)){
        scene->deleteObject(sList.at(1));
    }
}
