import * as functions from "firebase-functions"
import * as admin from "firebase-admin"
import * as express from "express"

admin.initializeApp()

const app = express()

const fcm = admin.messaging()

app.get("/", ( req, res ) => {
    res.send("HELLO WORLD EXPRESS")
})


app.get('/pop_user/:device_id/:difficulty', async (req, res) =>{
    const device_id = req.params.device_id
    const difficulty = req.params.difficulty
    try{
        const querrysnapshot = await admin.firestore().collection(`queue${device_id}`).orderBy("timestamp", "asc").limit(2).get()
        let promise_array:any[] = []
        if(querrysnapshot.docs.length >= 1){
            let current_user = querrysnapshot.docs[0].data()
            // deleting user from queue
            let p1 =  querrysnapshot.docs[0].ref.delete()
            // deleting device from users queue
            let p2 =  admin.firestore().collection('users').doc(current_user!.user).update({
                "queues": admin.firestore.FieldValue.arrayRemove(device_id)
            })
            // adding device to users history collection
            let p3 = admin.firestore().collection(`users/${current_user!.user}/history`).add({
                device: device_id,
                difficulty: difficulty,
                timestamp: admin.firestore.FieldValue.serverTimestamp()
            })
            promise_array.push(p1,p2,p3)
            await Promise.all(promise_array)
        }
        if(querrysnapshot.docs.length >= 2){
            let next_user = querrysnapshot.docs[1].data()
            
            // sending notification
            
            const payload: admin.messaging.MessagingPayload = {
                notification: {
                  title: 'its your turn!',
                  body: `its your turn on machine ${device_id}`,
                  click_action: 'FLUTTER_NOTIFICATION_CLICK' // required only for onResume or onLaunch callbacks
                }
            };
            await fcm.sendToDevice(next_user!.fcm_token,payload)

            res.send({user: next_user!.user})
        }
        else {
            res.send({user: 'empty'})
        }
    }
    catch(error){
        console.log(error)
        res.status(500).send(error)
    }
})

app.post('/add_user', async (req, res) => {
    const device_id = req.body.device_id
    const user = req.body.user
    let token = req.body.fcm_token
    try{
        const querrysnapshot = await admin.firestore().collection(`queue${device_id}`).where("user","==",user).get()
        if (querrysnapshot.docs.length > 0){
            res.send({result:"already in queue"})
        }
        else {
            if (token == "from machine"){
                let user_data = (await admin.firestore().collection('users').doc(user).get()).data()
                token = user_data!.fcm_token
            }
            let p1 = admin.firestore().collection(`queue${device_id}`).doc(user).set({
                user: user,
                fcm_token: token,
                timestamp: admin.firestore.FieldValue.serverTimestamp()
            })
            let p2 = admin.firestore().collection('users').doc(user).update({
                "queues": admin.firestore.FieldValue.arrayUnion(device_id)
            })
            let promise_array = []
            promise_array.push(p1,p2)
            await Promise.all(promise_array)
            res.send({result:"added to queue"})
        }
    }
    catch(error){
        console.log(error)
        res.status(500).send(error)
    }
})

exports.api = functions.https.onRequest(app)

// // Start writing Firebase Functions
// // https://firebase.google.com/docs/functions/typescript
//
// export const helloWorld = functions.https.onRequest((request, response) => {
//   functions.logger.info("Hello logs!", {structuredData: true});
//   response.send("Hello from Firebase!");
// });
