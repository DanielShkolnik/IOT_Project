import 'package:iotrain/models/user.dart';
import 'package:iotrain/screens/authenticate/authenticate.dart';
import 'package:iotrain/screens/home/home.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:firebase_auth/firebase_auth.dart';

class Wrapper extends StatelessWidget {

  FirebaseAuth auth = FirebaseAuth.instance;

  @override
  Widget build(BuildContext context) {

    final user = auth.authStateChanges()
    .listen((User user) {return user;});
    
    // return either the Home or Authenticate widget
    if (user == null){
      return Authenticate();
    } else {
      return Home();
    }
    
  }
}