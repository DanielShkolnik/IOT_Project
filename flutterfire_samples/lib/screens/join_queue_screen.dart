import 'package:flutter/material.dart';
import 'queue_status_screen.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';

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
              ElevatedButton(onPressed: () async {
                String? fcmToken = await _fcm.getToken();
                http.Response response = await http.post(
                Uri.parse('https://us-central1-iotrain-49a8d.cloudfunctions.net/api/add_user'),
                body: {
                  'device_id': '1',
                  'user': user.displayName! + "_" + user.uid,
                  'fcm_token': fcmToken
                },
                );
                var jsonResponse = jsonDecode(response.body);
                print(jsonResponse);

                // queue1.doc(user.uid).set({
                // 'timestamp': Timestamp.now(),
                // 'user':  user.displayName! + "_" + user.uid});
            },
            style: ElevatedButton.styleFrom(primary: Colors.orange),
            child: Text("Join Machine 1 queue", style: TextStyle(color: Colors.black, fontSize: 20))),
            ElevatedButton(onPressed: () {
          
            },
            style: ElevatedButton.styleFrom(primary: Colors.orange),
            child: Text("Join Machine 2 queue", style: TextStyle(color: Colors.black, fontSize: 20))),
          ])
        )
      )
    );
  }
}