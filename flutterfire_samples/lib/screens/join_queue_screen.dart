import 'package:flutter/material.dart';
import 'queue_status_screen.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';

class JoinQueueScreen extends StatelessWidget {
  final User user;
  final CollectionReference queue1 = FirebaseFirestore.instance.collection('queue1');
  JoinQueueScreen({required this.user});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.white),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Join Machine Queue"),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.list_rounded),
            tooltip: 'Queue status',
            onPressed: () {
              Navigator.push(context,
                MaterialPageRoute(
                  builder: (context) => QueueStatusScreen(uid: user.uid),
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
                queue1.doc(user.uid).set({
                'timestamp': Timestamp.now(),
                'user':  user.displayName! + "_" + user.uid
              });
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