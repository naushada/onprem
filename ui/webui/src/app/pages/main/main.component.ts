import { Component } from '@angular/core';
import { filter } from 'rxjs';
import { EventService } from 'src/common/event.service';

@Component({
  selector: 'app-main',
  templateUrl: './main.component.html',
  styleUrls: ['./main.component.scss']
})
export class MainComponent {

  selectedItem:string = "Config";
  role:string = "advanced";
  constructor(private event: EventService) {
    event.subscribe("user.login", (id:String, document:string) => {
      if(id == "user.login") {
        let response = JSON.parse(JSON.stringify(document));
        if(response["status"] == "success") {
          this.role = response["role"];
        } else {
          this.role = "basic";
        }
      }
    });
  }

  onClick(item:string) {
    this.selectedItem = item;
  }
}
