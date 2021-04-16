import 'package:flutter/material.dart';

class SchedualPage extends StatefulWidget {
  @override
  _SchedualPageState createState() => _SchedualPageState();
}

class _SchedualPageState extends State<SchedualPage> {
  var entries = <ListTile>[];
  void _addEntry() {
    setState(() => entries.add(ListTile(
            title: Text(
          DateTime.now().toString(),
        ))));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("My Schedual"),
      ),
      body: Column(children: <Widget>[
        Container(
          child: Column(
            children: entries,
          ),
        ),
      ]),
      floatingActionButton: FloatingActionButton(
        onPressed: _addEntry,
        tooltip: 'Schedual',
        child: Icon(Icons.add),
      ),
    );
  }
}
