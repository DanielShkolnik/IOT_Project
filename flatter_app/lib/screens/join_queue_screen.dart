import 'package:flutter/material.dart';
import 'queue_status_screen.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';
import 'queue.dart';

class JoinQueueScreen extends StatelessWidget {
  final User user;
  final CollectionReference queue1 = FirebaseFirestore.instance.collection('queue1');
  final FirebaseMessaging _fcm = FirebaseMessaging.instance;
  JoinQueueScreen({required this.user});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.greenAccent),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Join Machine Queue", style: TextStyle(color: Colors.greenAccent),),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.list_rounded, color: Colors.greenAccent,),
            tooltip: 'Queue status',
            onPressed: () {
              Navigator.push(context,
                MaterialPageRoute(
                  builder: (context) => QueueStatusScreen(user: user),
                ),
              );
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
                Navigator.push(context,
                MaterialPageRoute(
                  builder: (context) => QueueScreen(user: user, device: '1',),
                ),
              );
            },
            style: ElevatedButton.styleFrom(primary: Colors.orange),
            child: Text("Machine 1 queue", style: TextStyle(color: Colors.black, fontSize: 20))),
            ElevatedButton(onPressed: () {
              Navigator.push(context,
                MaterialPageRoute(
                  builder: (context) => QueueScreen(user: user, device: '2',),
                ),
              );
            },
            style: ElevatedButton.styleFrom(primary: Colors.orange),
            child: Text("Machine 2 queue", style: TextStyle(color: Colors.black, fontSize: 20))),
          ])
        )
      )
    );
  }
}