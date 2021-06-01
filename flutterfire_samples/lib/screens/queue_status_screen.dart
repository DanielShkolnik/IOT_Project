import 'package:flutter/material.dart';
import 'package:flutterfire_samples/res/custom_colors.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

class QueueStatusScreen extends StatefulWidget {
  final String uid;
  const QueueStatusScreen({required this.uid});

  @override
  _QueueStatusScreenState createState() => _QueueStatusScreenState();
}

class _QueueStatusScreenState extends State<QueueStatusScreen>{
  final Stream<QuerySnapshot> _queue1Stream = FirebaseFirestore.instance.collection('queue1').orderBy("timestamp", descending: false).snapshots();
  //final Stream<QuerySnapshot> _queue2Stream = FirebaseFirestore.instance.collection('queue2').snapshots();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.greenAccent),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Machine Queue Status", style: TextStyle(color: Colors.greenAccent)),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.list_rounded, color: Colors.greenAccent),
            tooltip: 'Queue status',
            onPressed: () {
              // handle the press
            },)],
        centerTitle: true,
      ),
      body: Container(
        padding: EdgeInsets.all(32),
        child: Center(
          child: StreamBuilder<QuerySnapshot>(
            stream: _queue1Stream,
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
                    color: (document.get('user').split("_")[1] == widget.uid) ? Colors.orange : Colors.grey,
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