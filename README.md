Qt TileServer Controller

This is a simple Qt application that allows you to start and stop a TileServer running inside Docker using UI buttons.

Features

Start TileServer using Docker

Stop TileServer using Docker

Simple Qt GUI (Start / Stop buttons)

Displays logs or status messages

Works with any TileServer (e.g., TileServer GL, Maptiler, custom servers)

How It Works

The Qt program runs Docker commands internally:

Start button
Runs a command similar to:

docker run -d -p 8080:8080 <tileserver-image>


Stop button
Stops the running container:

docker stop <container-id>


Qt executes these using QProcess in the backend.

How to Run

Install and start Docker .

Open this project in Qt Creator.

Build and run the Qt application.

Click Start to launch the TileServer.

Click Stop to stop it.
