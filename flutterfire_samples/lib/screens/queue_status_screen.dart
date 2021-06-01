import 'package:flutter/material.dart';

class QueueStatusScreen extends StatefulWidget {
  //final String uid;
  //const QueueStatusScreen({required this.uid});

  @override
  _QueueStatusScreenState createState() => _QueueStatusScreenState();
}

class _QueueStatusScreenState extends State<QueueStatusScreen>{
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.white),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Machine Queue Status"),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.list_rounded),
            tooltip: 'Queue status',
            onPressed: () {
              // handle the press
            },)],
        centerTitle: true,
      ),
      body: Container(
        padding: EdgeInsets.all(32),
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(onPressed: () {
              
            },
            child: Text("Join Machine 1 queue")),
            ElevatedButton(onPressed: () {
              
            },
            child: Text("Join Machine 2 queue")),
          ])
        )
      )
    );
  }
}