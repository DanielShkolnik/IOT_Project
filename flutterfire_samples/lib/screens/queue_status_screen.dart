import 'package:flutter/material.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_auth/firebase_auth.dart';

class QueueStatusScreen extends StatefulWidget {
  final User user;
  final CollectionReference users = FirebaseFirestore.instance.collection('users');
  final Stream<QuerySnapshot> queue1Stream = FirebaseFirestore.instance.collection('queue1').orderBy("timestamp", descending: false).snapshots();
  final Stream<QuerySnapshot> queue2Stream = FirebaseFirestore.instance.collection('queue2').orderBy("timestamp", descending: false).snapshots();
  late List<int> positions;

  QueueStatusScreen({required this.user});

  @override
  _QueueStatusScreenState createState() => _QueueStatusScreenState();
}

// List<Stream<QuerySnapshot>> get_streams(User user, DocumentReference userdoc){
//     var arr = userdoc.get('queues');
// }

// Widget getStreams(BuildContext context, Stream<QuerySnapshot> _userStream){
//   return StreamBuilder<QuerySnapshot>(
//       stream: _userStream,
//       builder: (BuildContext context, AsyncSnapshot<QuerySnapshot> snapshot) {
//         if (snapshot.hasError) {
//           return Text('Something went wrong');
//         }

//         if (snapshot.connectionState == ConnectionState.waiting) {
//           return Text("Loading");
//         }

//         return new ListView(
//           children: snapshot.data.docs.map((DocumentSnapshot document) {
//           Map<String, dynamic> data = document.data() as Map<String, dynamic>;
//             return new ListTile(
//               title: new Text(data['full_name']),
//               subtitle: new Text(data['company']),
//             );
//           }).toList(),
//         );
// }

class _QueueStatusScreenState extends State<QueueStatusScreen>{
  late Stream<DocumentSnapshot> user_stream;// = FirebaseFirestore.instance.collection('queue1').orderBy("timestamp", descending: false).snapshots();
  //final Stream<QuerySnapshot> _queue2Stream = FirebaseFirestore.instance.collection('queue2').snapshots();
  List<int> positions = [-1,-1];

  @override
  Widget build(BuildContext context) {
    user_stream = FirebaseFirestore.instance.collection('users').doc(widget.user.displayName! + "_" + widget.user.uid).snapshots();
    widget.queue1Stream.listen((snapshot) { 
      List<QueryDocumentSnapshot> list = snapshot.docs;
      int i;
      for(i = 0 ; i < list.length ; i++){
        if(list[i].get('user').split("_")[1] == widget.user.uid) break;
      }
      positions[0] =  i;
      //setState(()=>{positions[0] =  i});
      print("1 pos: " + positions[0].toString());
    });
    widget.queue2Stream.listen((snapshot) { 
      List<QueryDocumentSnapshot> list = snapshot.docs;
      int i;
      for(i = 0 ; i < list.length ; i++){
        if(list[i].get('user').split("_")[1] == widget.user.uid) break;
      }
      positions[1] =  i;
       //setState(()=>{positions[1] =  i});
       print("2 pos: " + positions[1].toString());
    });
    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
            icon: Icon(Icons.arrow_back, color: Colors.greenAccent),
            onPressed: () => Navigator.of(context).pop(),
          ),
        title: Text("Machine Queue Status", style: TextStyle(color: Colors.greenAccent)),
        actions: <Widget>[
          // IconButton(
          //   icon: const Icon(Icons.list_rounded, color: Colors.greenAccent),
          //   tooltip: 'Queue status',
          //   onPressed: () {
          //   },)
          ],
        centerTitle: true,
      ),
      body: Container(
        padding: EdgeInsets.all(32),
        child: Center(
          child: StreamBuilder<DocumentSnapshot>(
            stream: user_stream,
            builder: (BuildContext context, AsyncSnapshot<DocumentSnapshot> snapshot) {
              if (snapshot.hasError) {
                return Text('Something went wrong');
              }

              if (snapshot.connectionState == ConnectionState.waiting) {
                return Text("Loading");
              }

              return new ListView(
                children: snapshot.data!['queues'].map<Widget>((device) {
                  
                  return new Container(
                    child:ListTile(
                    title: new Text('queueing for machine '+ device, style: TextStyle(color: Colors.black, fontSize: 20, fontWeight: FontWeight.bold),),
                    subtitle: new Text('position in queue: ' + positions[int.parse(device)-1].toString(), style: TextStyle(color: Colors.black,),),
                    ),
                    height: 70,
                    color: (positions[int.parse(device)-1] <= 1) ? Colors.orange : Colors.grey,
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