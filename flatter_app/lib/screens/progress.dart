import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_messaging/firebase_messaging.dart';

class ProgressScreen extends StatefulWidget {
  final User user;
  final FirebaseMessaging fcm = FirebaseMessaging.instance;
  ProgressScreen({required this.user});

  @override
  _ProgressScreenState createState() => _ProgressScreenState();
}

class _ProgressScreenState extends State<ProgressScreen>{
  late Stream<QuerySnapshot> historyStreamDevice;
  final List<String> filters = ["machine 1", "machine 2"];
  late Query<Map<String, dynamic>> history;
  String? device;

  @override
  Widget build(BuildContext context) {
    history = FirebaseFirestore.instance.collection('users').doc(widget.user.displayName! + "_" + widget.user.uid).collection('history').orderBy("timestamp", descending: false);
    historyStreamDevice = (device != null) ? history.where('device', isEqualTo: device!.split(' ')[1]).snapshots() : history.snapshots();

    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.greenAccent),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("My progress", style: TextStyle(color: Colors.greenAccent)),
        actions: <Widget>[
          DropdownButton<String>(
          items: filters.map((String val) {
                   return new DropdownMenuItem<String>(
                        value: val,
                        child: new Text(val, style: TextStyle(color: (device == val) ? Colors.orange : Colors.grey)),
                         );
                    }).toList(),
          hint: Text("Filter by"),
          onChanged: (val) {
                  this.setState(() {
                    if(device == null)device = val;
                    else if (device != val)device = val;
                    else device = null;
                    });
                  })],
        centerTitle: true,
      ),
      body: Container(
        padding: EdgeInsets.all(32),
        child: Center(
          child: StreamBuilder<QuerySnapshot>(
            stream: historyStreamDevice,
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
                    title: new Text("Device "+document.get('device')+": Difficulty - "+document.get('difficulty'), style: TextStyle(color: Colors.black, fontSize: 20, fontWeight: FontWeight.bold),),
                    subtitle: new Text(document.get('timestamp').toDate().toString(), style: TextStyle(color: Colors.black,),),
                    ),
                    height: 70,
                    color: Colors.orange[300],
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