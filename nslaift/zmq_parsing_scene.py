import zmq


def send_zmq_msg(msg):
    socket.send(msg)
    return socket.recv()


if __name__ == '__main__':
    context = zmq.Context()

    # Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

    send_zmq_msg("createObject;cam1;camera")
    send_zmq_msg("manipulateObject;cam1;translate;0.0,0.0,0.0")
    send_zmq_msg("createObject;cam2;camera")
    send_zmq_msg("manipulateObject;cam2;translate;0.0,-7.0,-3.5")
    send_zmq_msg("manipulateObject;cam2;spin;-45.0,0.0,0.0")
    send_zmq_msg("createObject;sphere1;sphere")
    send_zmq_msg("manipulateObject;sphere1;translate;0.0,-0.1,2.0")
    send_zmq_msg("render")

    send_zmq_msg("createObject;sphere2;sphere")
    send_zmq_msg("manipulateObject;sphere2;translate;0.0,0.0,9.0")
    send_zmq_msg("manipulateObject;sphere2;radius;4")
    # send_zmq_msg("manipulateObject;sphere2;setmaterial;blank")
    send_zmq_msg("render")

    # send_zmq_msg("deleteObject;cam1")
    send_zmq_msg("manipulateObject;sphere2;radius;3")
    send_zmq_msg("createObject;sphere3;sphere")
    send_zmq_msg("manipulateObject;sphere3;translate; 0.3, 0.0, 2.0")
    # send_zmq_msg("manipulateObject;sphere2;setmaterial;normal")
    send_zmq_msg("render")

    send_zmq_msg("manipulateObject;sphere3;translate; 0.1, 0.2, 0.0")
    send_zmq_msg("render")

    # send_zmq_msg("createObject;cam1;camera")
    # send_zmq_msg("manipulateObject;cam1;translate;0.0,0.0,1.0")
    # send_zmq_msg("deleteObject;sphere3")
    # send_zmq_msg("render")
    #
    # send_zmq_msg("deleteObject;sphere1")
    # send_zmq_msg("render")
