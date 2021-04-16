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
      appBar: AppBar(title: const Text('My schedual')),
      body: entries.length > 0
          ? ListView.builder(
              itemCount: entries.length,
              itemBuilder: (BuildContext context, int index) {
                return Container(
                  child: ListTile(
                    title: Text(DateTime.now().toString()),
                    tileColor: Colors.grey,
                  ),
                  margin: EdgeInsets.all(10),
                );
              },
            )
          : Center(child: const Text('No session scheduald')),
      floatingActionButton: FloatingActionButton(
        onPressed: _addEntry,
        tooltip: 'Schedual',
        child: Icon(Icons.add),
      ),
    );
  }
}
