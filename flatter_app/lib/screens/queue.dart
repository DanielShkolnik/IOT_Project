import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:http/http.dart' as http;
import 'package:firebase_messaging/firebase_messaging.dart';
import 'dart:convert';

class QueueScreen extends StatefulWidget {
  final User user;
  final String device;
  final FirebaseMessaging fcm = FirebaseMessaging.instance;
  QueueScreen({required this.user, required this.device});

  @override
  _QueueScreenState createState() => _QueueScreenState();
}

class _QueueScreenState extends State<QueueScreen>{
  late CollectionReference queueref;
  late Stream<QuerySnapshot> _queueStream;// = FirebaseFirestore.instance.collection(widget.queue).orderBy("timestamp", descending: false).snapshots();
  //final Stream<QuerySnapshot> _queue2Stream = FirebaseFirestore.instance.collection('queue2').snapshots();

  @override
  Widget build(BuildContext context) {
    queueref = FirebaseFirestore.instance.collection('queue' + widget.device);
    _queueStream = FirebaseFirestore.instance.collection('queue' + widget.device).orderBy("timestamp", descending: false).snapshots();
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.greenAccent),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Machine Queue Status", style: TextStyle(color: Colors.greenAccent)),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.plus_one_outlined, color: Colors.greenAccent),
            tooltip: 'Join Queue',
            onPressed: () async {
                String? fcmToken = await widget.fcm.getToken();
                http.Response response = await http.post(
                Uri.parse('https://us-central1-iotrain-49a8d.cloudfunctions.net/api/add_user'),
                body: {
                  'device_id': widget.device,
                  'user': widget.user.displayName! + "_" + widget.user.uid,
                  'fcm_token': fcmToken
                },
                );
                var jsonResponse = jsonDecode(response.body);
                print(jsonResponse);
              },)],
        centerTitle: true,
      ),
      body: Container(
        padding: EdgeInsets.all(32),
        child: Center(
          child: StreamBuilder<QuerySnapshot>(
            stream: _queueStream,
            builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
              if (snapshot.hasError) {
                return Text('Something went wrong');
              }

              if (snapshot.connectionState == ConnectionState.waiting) {
                return Text("Loading");
              }

              return new ListView(
                children: snapshot.data!.docs.map((DocumentSnapshot document) {
                  return new Container(
                    child:ListTile(
                    title: new Text(document.get('user').split("_")[0], style: TextStyle(color: Colors.black, fontSize: 20, fontWeight: FontWeight.bold),),
                    subtitle: new Text(document.get('timestamp').toDate().toString(), style: TextStyle(color: Colors.black,),),
                    ),
                    height: 70,
                    color: (document.get('user').split("_")[1] == widget.user.uid) ? Colors.orange : Colors.grey,
                    margin: EdgeInsets.all(10.0),
                  );
                }).toList(),
              );
            })
          )
      )
    );
  }
}