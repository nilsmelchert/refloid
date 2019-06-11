import zmq


def send_zmq_msg(msg):
    socket.send(msg)
    return socket.recv()


if __name__ == '__main__':
    context = zmq.Context()

    # Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")
    send_zmq_msg("clear")

    send_zmq_msg("createObject;lightpoint1;lightpoint")
    send_zmq_msg("manipulateObject;lightpoint1;translate;10.0,0.0,0.0")
    send_zmq_msg("manipulateObject;lightpoint1;color;1.0,0.0,0.0")

    send_zmq_msg("createObject;cam1;camera")
    send_zmq_msg("manipulateObject;cam1;translate;0.0,0.0,0.0")

    # send_zmq_msg("createObject;cam2;camera")
    # send_zmq_msg("manipulateObject;cam2;translate;0.0,-7.0,-3.5")
    # send_zmq_msg("manipulateObject;cam2;spin;-45.0,0.0,0.0")
    send_zmq_msg("createObject;sphere1;sphere")
    send_zmq_msg("manipulateObject;sphere1;translate;0.0,-0.1,2.0")
    send_zmq_msg("render")

    send_zmq_msg("manipulateObject;sphere1;setMaterialParameter;Ks;1.0,1.0,1.0")
    send_zmq_msg("render")
    send_zmq_msg("manipulateObject;sphere1;setMaterialParameter;Ks;0.0,0.0,0.0")
    send_zmq_msg("render")

    # send_zmq_msg("clear")
    #
    # send_zmq_msg("createObject;lightpoint1;lightpoint")
    # send_zmq_msg("manipulateObject;lightpoint1;translate;10.0,0.0,0.0")
    # send_zmq_msg("manipulateObject;lightpoint1;color;0.0,0.0,0.5")
    # send_zmq_msg("createObject;lightpoint2;lightpoint")
    # send_zmq_msg("manipulateObject;lightpoint2;translate;0.0,01.0,0.0")
    # send_zmq_msg("manipulateObject;lightpoint2;color;0.0,0.5,0.0")
    # send_zmq_msg("createObject;cam1;camera")
    # send_zmq_msg("manipulateObject;cam1;translate;0.0,0.0,0.0")
    #
    # send_zmq_msg("createObject;sphere2;sphere")
    # send_zmq_msg("manipulateObject;sphere2;translate;0.1,-0.1,2.0")
    # send_zmq_msg("render")
    #
    # send_zmq_msg("manipulateObject;sphere2;translate;0.1,-0.1,2.0")
    # send_zmq_msg("manipulateObject;sphere2;material;normal")
    # send_zmq_msg("render")
    #
    # send_zmq_msg("manipulateObject;sphere2;translate;0.1,-0.1,2.0")
    # send_zmq_msg("render")
